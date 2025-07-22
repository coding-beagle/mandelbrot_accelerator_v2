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
#include <xgpio.h>
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

int 	StartMandelbrotting(XGpio *MandelbrotGPIOAddress) {
    XGpio_DiscreteWrite(MandelbrotGPIOAddress, 1, 1);
    // for(volatile int i = 0; i < 100; i++){}
    XGpio_DiscreteClear(MandelbrotGPIOAddress, 1, 1);
    return XST_SUCCESS;
}

int getIterations(XGpio *MandelbrotGPIOAddress) {
    u8 finished = 0;
    u32 iterCount, value;

    iterCount = 0;

    while (finished == 0){
        value = XGpio_DiscreteRead(MandelbrotGPIOAddress, 1);
        iterCount = (value & 0b1111111111);
        finished = (value & 1024) >> 10;
    }
    
    return iterCount;
}

int64_t convert_hex_bytes_to_val(char * value) {
    int64_t value_out = 0;
    for (int i = 0; i < 8; i++){
        value_out <<= 8;
        value_out |= value[i];
    }
    return value_out;
}

int SetUpGPIO(XGpio *GPIOAddress, int ID, u8 in){
    XGpio_Initialize(GPIOAddress, ID);
    if(in == 1){
        XGpio_SetDataDirection(GPIOAddress, 1, 0xFFFFFFFF);
        XGpio_SetDataDirection(GPIOAddress, 2, 0xFFFFFFFF);
    } else if(in == 0){
        XGpio_SetDataDirection(GPIOAddress, 1, 0x00000000);
        XGpio_SetDataDirection(GPIOAddress, 2, 0x00000000);
    } else {
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

int ReadGPIO(XGpio *GPIOAddress, int64_t *dataBuffer){
    uint64_t high = XGpio_DiscreteRead(GPIOAddress, 1);
    uint64_t low = XGpio_DiscreteRead(GPIOAddress, 2);
    *dataBuffer = (high << 32) | low;
    return XST_SUCCESS;
}

int SetGPIO(XGpio *GPIOAddress, int64_t value){
    XGpio_DiscreteClear(GPIOAddress, 1, 0xFFFFFFFF);
    XGpio_DiscreteClear(GPIOAddress, 2, 0xFFFFFFFF);
    XGpio_DiscreteWrite(GPIOAddress, 1, (u32)(value));
    XGpio_DiscreteWrite(GPIOAddress, 2, (u32)(value >> 32));
    return XST_SUCCESS;
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

XGpio CurrentX;
XGpio CurrentY;
XGpio XStep;
XGpio YStep;
XGpio TopLeftX;
XGpio TopLeftY;
// XGpio ZRe;
// XGpio ZIm;
XGpio Start_Mandelbrot;
XGpio Mandelbrot1;
XGpio Mandelbrot2;
XGpio Mandelbrot3;
XGpio Mandelbrot4;
XGpio Mandelbrot5;
XGpio Mandelbrot6;
XGpio Mandelbrot7;
XGpio Mandelbrot8;

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
                SetGPIO(&TopLeftX, top_left_x_val);
                
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
                SetGPIO(&TopLeftY, top_left_y_val);
                
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
                SetGPIO(&XStep, x_step_val);
                
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
                SetGPIO(&YStep, y_step_val);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(y_step_val >> 32), 
                                (uint32_t)(y_step_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } else if (strstr((char *)p->payload, "CURRX") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                current_x_val= convert_hex_bytes_to_val(val);
                SetGPIO(&CurrentX, current_x_val);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(current_x_val >> 32), 
                                (uint32_t)(current_x_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        } else if (strstr((char *)p->payload, "CURRY") != NULL){
            if(p->len > 14){
                for (int i = 0; i < 8; i++){
                    val[i] = ((char *)p->payload)[i + 6];
                }
                val[8] = '\0';
                current_y_val= convert_hex_bytes_to_val(val);
                SetGPIO(&CurrentY, current_y_val);
                
                err = tcp_write(tpcb, val,8, 1);
            } else {
                sprintf(val, "%08x%08x", (uint32_t)(current_y_val >> 32), 
                                (uint32_t)(current_y_val & 0xFFFFFFFF));
                tcp_write(tpcb, val,16, 1);
            }
        // } else if (strstr((char *)p->payload, "ZREAL") != NULL){
        //     xil_printf("This block now!");
        //     ReadGPIO(&ZRe, &real_z);
        //     xil_printf("ZReal value: %08x%08x", (uint32_t)(real_z >> 32), 
        //                     (uint32_t)(real_z & 0xFFFFFFFF));
        //     sprintf(val, "%08x%08x", (uint32_t)(real_z >> 32), 
        //                     (uint32_t)(real_z & 0xFFFFFFFF));
        //     tcp_write(tpcb, val,16, 1);
            
        // } else if (strstr((char *)p->payload, "ZIMAG") != NULL){
        //     ReadGPIO(&ZIm, &im_z);
        //     sprintf(val, "%08x%08x", (uint32_t)(im_z >> 32), 
        //                     (uint32_t)(im_z & 0xFFFFFFFF));
        //     tcp_write(tpcb, val,16, 1);

        
        // full frame buffer calcualtion
        } else if (strstr((char *)p->payload, "CALCE") != NULL) {
            
            xil_printf("STARTING");

            for(int i = 0; i < HEIGHT; i++ ){
                for(int j = 0; j < LINE_SIZE; j += 4) {
					// xil_printf("Calculating pixel x, y = %d, %d", i, j);
                    StartMandelbrotting(&Start_Mandelbrot);
					
                    u32 value1 = getIterations(&Mandelbrot1);
					// xil_printf("Value 1 = %d", value1);

                    u32 value2 = getIterations(&Mandelbrot2);
					// xil_printf("Value 2 = %d", value2);

                    u32 value3 = getIterations(&Mandelbrot3);
					// xil_printf("Value 3 = %d", value3);

                    u32 value4 = getIterations(&Mandelbrot4);
					// xil_printf("Value 4 = %d", value4);

					// u32 value5 = getIterations(&Mandelbrot5);
					// xil_printf("Value 5 = %d", value5);

                    // u32 value6 = getIterations(&Mandelbrot6);
					// xil_printf("Value 6 = %d", value6);

                    // u32 value7 = getIterations(&Mandelbrot7);
					// xil_printf("Value 7 = %d", value7);

                    // u32 value8 = getIterations(&Mandelbrot8);
					// xil_printf("Value 8 = %d", value8);

                    current_x_val += 0x0040000000000000;
                    SetGPIO(&CurrentX, current_x_val);
                    framebuffer[j + i * LINE_SIZE] = (u8)(value1);
                    framebuffer[j + 1 + i * LINE_SIZE] = (u8)(value2);
                    framebuffer[j + 2 + i * LINE_SIZE] = (u8)(value3);
                    framebuffer[j + 3 + i * LINE_SIZE] = (u8)(value4);
					// framebuffer[j + 4 + i * LINE_SIZE] = (u8)(value5);
                    // framebuffer[j + 5 + i * LINE_SIZE] = (u8)(value6);
                    // framebuffer[j + 6 + i * LINE_SIZE] = (u8)(value7);
                    // framebuffer[j + 7 + i * LINE_SIZE] = (u8)(value8);
                }

                current_x_val = 0;
                current_y_val += 0x0010000000000000;
                SetGPIO(&CurrentX, current_x_val);
                SetGPIO(&CurrentY, current_y_val);
            }

            xil_printf("Finished!");
            tcp_write(tpcb, "FINISHED",8, 1);

        // smaller calculation
        // } 
		// else if (strstr((char *)p->payload, "CALCL") != NULL) {
            
        //     xil_printf("STARTING LOW RES");

        //     for(int i = 0; i < HEIGHT_LOW_RES; i++ ){
        //         for(int j = 0; j < LINE_SIZE_LOW_RES; j += 4) {
        //             StartMandelbrotting(&Start_Mandelbrot);
        //             u32 value1 = getIterations(&Mandelbrot1);
        //             u32 value2 = getIterations(&Mandelbrot2);
        //             u32 value3 = getIterations(&Mandelbrot3);
        //             u32 value4 = getIterations(&Mandelbrot4);
        //             current_x_val += 0x0040000000000000;
        //             SetGPIO(&CurrentX, current_x_val);
        //             framebuffer[j + i * LINE_SIZE_LOW_RES] = (u8)(value1);
        //             framebuffer[j + 1 + i * LINE_SIZE_LOW_RES] = (u8)(value2);
        //             framebuffer[j + 2 + i * LINE_SIZE_LOW_RES] = (u8)(value3);
        //             framebuffer[j + 3 + i * LINE_SIZE_LOW_RES] = (u8)(value4);
        //         }

        //         current_x_val = 0;
        //         current_y_val += 0x0010000000000000;
        //         SetGPIO(&CurrentX, current_x_val);
        //         SetGPIO(&CurrentY, current_y_val);
        //     }

        //     xil_printf("FINISHED LOW RES CALC");
        //     tcp_write(tpcb, "FINISHED",8, 1);

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

int SetUpMandelbrot(XGpio *mandelbrot, UINTPTR addr) {
	err_t res = XST_SUCCESS;
    res = XGpio_Initialize(mandelbrot, addr);
    XGpio_SetDataDirection(mandelbrot, 1, 0b011111111111);
    return (int)res;
}

int start_application()
{
	struct tcp_pcb *pcb;
	err_t err;
	unsigned port = 7;

    SetUpGPIO(&CurrentX, XPAR_PIXEL_X_BASEADDR, 0);
    SetUpGPIO(&CurrentY, XPAR_PIXEL_Y_BASEADDR, 0);
    SetUpGPIO(&XStep, XPAR_STEP_X_BASEADDR, 0);
    SetUpGPIO(&YStep, XPAR_STEP_Y_BASEADDR, 0);
    SetUpGPIO(&TopLeftX, XPAR_TOP_LEFT_X_BASEADDR, 0);
    SetUpGPIO(&TopLeftY, XPAR_TOP_LEFT_Y_BASEADDR, 0);
    // SetUpGPIO(&ZRe, XPAR_Z_RE_BASEADDR, 1);
    // SetUpGPIO(&ZIm, XPAR_Z_IM_BASEADDR, 1);

    XGpio_Initialize(&Start_Mandelbrot, XPAR_START_MANDELBROT_BASEADDR);
    XGpio_SetDataDirection(&Start_Mandelbrot, 1, 0);
    
	int status = 0;

    status += SetUpMandelbrot(&Mandelbrot1, XPAR_MANDELBROT_0_BASEADDR);
    status += SetUpMandelbrot(&Mandelbrot2, XPAR_MANDELBROT_1_BASEADDR);
    status += SetUpMandelbrot(&Mandelbrot3, XPAR_MANDELBROT_2_BASEADDR);
    status += SetUpMandelbrot(&Mandelbrot4, XPAR_MANDELBROT_3_BASEADDR);
	// status += SetUpMandelbrot(&Mandelbrot5, XPAR_MANDELBROT_5_BASEADDR);
    // status += SetUpMandelbrot(&Mandelbrot6, XPAR_MANDELBROT_6_BASEADDR);
    // status += SetUpMandelbrot(&Mandelbrot7, XPAR_MANDELBROT_7_BASEADDR);
    // status += SetUpMandelbrot(&Mandelbrot8, XPAR_MANDELBROT_8_BASEADDR);

	if(status != 0){
		xil_printf("Inited wrong %d", status);
	} else {
		xil_printf("Inited Goodge!");
	}

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
