/*
 * Copyright (C) 2009 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <arch/cc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xparameters.h>
#include <xstatus.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#endif

#define LINE_SIZE 640
#define HEIGHT 480
#define LINES_PER_SEND 4

typedef struct {
    volatile uint32_t step_x_high;        // 0x00
    volatile uint32_t step_x_low;         // 0x04
    volatile uint32_t step_y_high;        // 0x08
    volatile uint32_t step_y_low;         // 0x0C
    volatile uint32_t top_left_x_high;    // 0x10
    volatile uint32_t top_left_x_low;     // 0x14
    volatile uint32_t top_left_y_high;    // 0x18
    volatile uint32_t top_left_y_low;     // 0x1C
    volatile uint32_t pixel_xy_1;         // 0x20
    volatile uint32_t pixel_xy_2;         // 0x24
    volatile uint32_t pixel_xy_3;         // 0x28
    volatile uint32_t pixel_xy_4;         // 0x2C
    volatile uint32_t start;              // 0x30
    volatile uint32_t reserved[2];        // 0x34, 0x38
    volatile uint32_t finished;           // 0x3C
    volatile uint32_t iter_count_1;       // 0x40
    volatile uint32_t iter_count_2;       // 0x44
    volatile uint32_t iter_count_3;       // 0x48
    volatile uint32_t iter_count_4;       // 0x4C
} mandelbrot_regs_t;

mandelbrot_regs_t *mandelbrot = (mandelbrot_regs_t *)XPAR_AXI_BROT_0_BASEADDR;


int transfer_data() {
	return 0;
}

void print_app_header()
{
#if (LWIP_IPV6==0)
	xil_printf("\n\r\n\r-----lwIP TCP echo server ------\n\r");
#else
	xil_printf("\n\r\n\r-----lwIPv6 TCP echo server ------\n\r");
#endif
	xil_printf("TCP packets sent to port 6001 will be echoed back\n\r");
}

int	StartMandelbrotting() {
    mandelbrot->start = 0b1111;
    return 0;
}

int getIterations(int mandel_num) {

	int bit_pos = 3 - mandel_num;  

    u8 finished_val = (mandelbrot->finished >> bit_pos) & 1;
    while (finished_val == 0) {
        finished_val = (mandelbrot->finished >> bit_pos) & 1;
    }

	switch (mandel_num) {
		case 0:
			return mandelbrot->iter_count_1;
			break;
		case 1:
			return mandelbrot->iter_count_2;
			break;
		case 2:
			return mandelbrot->iter_count_3;
			break;
		case 3:
			return mandelbrot->iter_count_4;
			break;
		default:
			return 0;
	} 
}

int64_t convert_hex_bytes_to_val(char * value) {
    int64_t value_out = 0;
    for (int i = 0; i < 8; i++){
        value_out <<= 8;
        value_out |= value[i];
    }
    return value_out;
}

int64_t current_x_val = 0;
int64_t current_y_val = 0;
int64_t x_step_val = 0;
int64_t y_step_val = 0;
int64_t top_left_x_val = 0;
int64_t top_left_y_val = 0;
int64_t real_z = 0;
int64_t im_z = 0;

int current_send = 0;

u8 linebuffer [LINE_SIZE] = {};
u8 framebuffer [LINE_SIZE * HEIGHT] = {};
char mandelbrot_send_buffer[LINE_SIZE * 2]; // 2 chars per byte + null terminator

u8 streaming = 0;

// Add these state variables at the top with your other globals
typedef struct {
    int current_line;
    int total_lines;
    int bytes_per_line_hex;  // LINE_SIZE * 2 (hex encoding)
    int is_streaming;
    int streaming_low_res;
    struct tcp_pcb *streaming_pcb;
} streaming_state_t;

static streaming_state_t stream_state = {0, 0, 0, 0, 0, NULL};


void write_uint64_to_reg(uint64_t value, volatile uint32_t *high) {
    *high = (uint32_t)(value >> 32);        // Upper 32 bits
    *(high + 1) = (uint32_t)(value & 0xFFFFFFFF);  // Lower 32 bits to next register
}


// Send a single chunk of the frame buffer. A chunk is LINE_SIZE * LINES_PER_SEND, i.e. n amount of lines
err_t send_next_chunk(struct tcp_pcb *tpcb)
{
    if (stream_state.current_line >= stream_state.total_lines) {
        return ERR_OK;
    }
    
    // Check available send buffer space
    u16_t available_space = tcp_sndbuf(tpcb);
    u16_t required_space = stream_state.bytes_per_line_hex + 32; // Extra for line header
    
    if (available_space < required_space) {
        // Not enough space, wait for sent_callback
        return ERR_OK;
    }
    
    
    // Prepare line data with header
    static char chunk_buffer[LINE_SIZE * LINES_PER_SEND * 2 + 128]; // Hex data + header space
    char *ptr = chunk_buffer;

    for (int line = 0; line < LINES_PER_SEND; line++ ){
        ptr += sprintf(ptr, "LINE:%04d:", stream_state.current_line + line);
        // Convert binary data to hex
        for (int i = 0; i < LINE_SIZE; i++) {
            ptr += sprintf(ptr, "%02x", framebuffer[i + (stream_state.current_line + line) * LINE_SIZE]);
        }

        // Add line terminator
        ptr += sprintf(ptr, "\n");
    }

    int total_length = ptr - chunk_buffer;
    
    // Send the line
    err_t err = tcp_write(tpcb, chunk_buffer, total_length, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        tcp_output(tpcb);
        stream_state.current_line += LINES_PER_SEND;
        xil_printf("Sent line %d/%d\n\r", stream_state.current_line, stream_state.total_lines);
    } else {
        xil_printf("Error sending line %d: %d\n\r", stream_state.current_line, (int)err);
    }

    return err;
    
}

// Callback for when data is successfully sent
err_t sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    if (!stream_state.is_streaming || tpcb != stream_state.streaming_pcb) {
        return ERR_OK;
    }
    
    // Continue streaming next line if there are more to send
    if (stream_state.current_line < stream_state.total_lines) {
        send_next_chunk(tpcb);
    } else {
        // Streaming complete
        stream_state.is_streaming = 0;
        stream_state.streaming_pcb = NULL;
        tcp_write(tpcb, "STREAM_COMPLETE\n", 16, 1);
        tcp_output(tpcb);
        xil_printf("Mandelbrot streaming complete\n\r");
    }
    
    return ERR_OK;
}

// Initialize streaming for MANDE command
err_t start_mandelbrot_streaming(struct tcp_pcb *tpcb)
{
    if (stream_state.is_streaming) {
        tcp_write(tpcb, "ERROR:ALREADY_STREAMING\n", 24, 1);
        return ERR_ALREADY;
    }
    
    // Initialize streaming state
    stream_state.current_line = 0;

    if(stream_state.streaming_low_res == 1){
        // stream_state.total_lines = HEIGHT_LOW_RES;
        // stream_state.bytes_per_line_hex = LINE_SIZE_LOW_RES * LINES_PER_SEND_LOW_RES * 2;
    } else {
        stream_state.total_lines = HEIGHT;
        stream_state.bytes_per_line_hex = LINE_SIZE * LINES_PER_SEND * 2;
    }
    stream_state.is_streaming = 1;
    stream_state.streaming_pcb = tpcb;
    
    // Set up the sent callback to continue streaming
    tcp_sent(tpcb, sent_callback);
    
    // Send initial response
    char header[64];
    sprintf(header, "STREAM_START:LINES:%d\n", stream_state.total_lines);
    tcp_write(tpcb, header, strlen(header), 1);
    tcp_output(tpcb);
    
    // Start sending first line
    return send_next_chunk(tpcb);
}

uint32_t convert_two_ints_to_packed12(uint32_t x, uint32_t y){
	return (uint32_t)(((y & 0x0FFF) << 12) | (x & 0x0FFF));
}


err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

	/* echo back the payload */
	/* in this case, we assume that the payload is < TCP_SND_BUF */
	if (tcp_sndbuf(tpcb) > p->len) {
        char val [18] = {};
        if(strstr((char *)p->payload, "X_TOP") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                top_left_x_val= convert_hex_bytes_to_val(val);
				write_uint64_to_reg(top_left_x_val, &mandelbrot->top_left_x_high);
                
                
                // xil_printf("Value converted: %08x%08x", (uint32_t)(value_inted >> 32), (uint32_t)(value_inted & 0xFFFFFFFF));
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(top_left_x_val >> 32), 
                                (uint32_t)(top_left_x_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } else if (strstr((char *)p->payload, "Y_TOP") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                top_left_y_val= convert_hex_bytes_to_val(val);
                write_uint64_to_reg(top_left_y_val, &mandelbrot->top_left_y_high);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(top_left_y_val >> 32), 
                                (uint32_t)(top_left_y_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } else if (strstr((char *)p->payload, "XSTEP") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                x_step_val= convert_hex_bytes_to_val(val);
                write_uint64_to_reg(x_step_val, &mandelbrot->step_x_high);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(x_step_val >> 32), 
                                (uint32_t)(x_step_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } else if (strstr((char *)p->payload, "YSTEP") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                y_step_val= convert_hex_bytes_to_val(val);
                write_uint64_to_reg(y_step_val, &mandelbrot->step_y_high);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(y_step_val >> 32), 
                                (uint32_t)(y_step_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } 
        // full frame buffer calcualtion
        else if (strstr((char *)p->payload, "CALCE") != NULL) {
            
            xil_printf("STARTING");
            for(int i = 0; i < HEIGHT; i++ ){
                for(int j = 0; j < LINE_SIZE; j += 4) {
                    mandelbrot->start = 0b1111;
					mandelbrot->start = 0b0000;

					mandelbrot->pixel_xy_1 = convert_two_ints_to_packed12(j,i);
					mandelbrot->pixel_xy_2 = convert_two_ints_to_packed12(j + 1,i);
					mandelbrot->pixel_xy_3 = convert_two_ints_to_packed12(j + 2,i);
					mandelbrot->pixel_xy_4 = convert_two_ints_to_packed12(j + 3,i);
					
                    u32 value1 = getIterations(0);
					// xil_printf("Value 1 = %d", value1);

                    u32 value2 = getIterations(1);
					// xil_printf("Value 2 = %d", value2);

                    u32 value3 = getIterations(2);
					// xil_printf("Value 3 = %d", value3);

                    u32 value4 = getIterations(3);


                    
                    framebuffer[j + i * LINE_SIZE] = (u8)(value1);
                    framebuffer[j + 1 + i * LINE_SIZE] = (u8)(value2);
                    framebuffer[j + 2 + i * LINE_SIZE] = (u8)(value3);
                    framebuffer[j + 3 + i * LINE_SIZE] = (u8)(value4);
                }
            }

            xil_printf("Finished!");
            tcp_write(tpcb, "FINISHED",8, 1);

        } else if (strstr((char *)p->payload, "MANDE") != NULL) {
            if(current_send > HEIGHT){
                return ERR_OK;
            }
            
            for(int line = 0; line < LINES_PER_SEND; line++ ){
                char *ptr = mandelbrot_send_buffer;

                for (int i = 0; i < LINE_SIZE; i++) {
                    ptr += sprintf(ptr, "%02x", framebuffer[i + (line + current_send) * LINE_SIZE]);
                }

                err = tcp_write(tpcb, mandelbrot_send_buffer, sizeof(mandelbrot_send_buffer), 1);       
                tcp_output(tpcb);
                
                xil_printf("Error code = %d", (int)err);
            }

            current_send += LINES_PER_SEND;

        } else if (strstr((char *)p->payload, "STREM") != NULL) {
            stream_state.streaming_low_res = 0;
            err = start_mandelbrot_streaming(tpcb);
        } else if (strstr((char *)p->payload, "LOWRE") != NULL) {
            stream_state.streaming_low_res = 0;
            err = start_mandelbrot_streaming(tpcb);
        } else if (strstr((char *)p->payload, "RESET") != NULL) {
            current_send = 0;
			mandelbrot->pixel_xy_1 = convert_two_ints_to_packed12(0,0);
			mandelbrot->pixel_xy_2 = convert_two_ints_to_packed12(0,0);
			mandelbrot->pixel_xy_3 = convert_two_ints_to_packed12(0,0);
			mandelbrot->pixel_xy_4 = convert_two_ints_to_packed12(0,0);

        } else {
            err = tcp_write(tpcb, p->payload, p->len, 1);
        }
		
	} else
		xil_printf("no space in tcp_sndbuf\n\r");

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	return ERR_OK;
}


int start_application()
{
	struct tcp_pcb *pcb;
	err_t err;
	unsigned port = 7;

	xil_printf("\n\r\n\rLET'S GO!!!\n\r\n\r");
    
	/* create new TCP PCB structure */
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ANY_TYPE, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP echo server started @ port %d\n\r", port);

	return 0;
}
