#include "../default.h"

void open_devices(){
    char paths[MAX_DEVICE_NUMBER][50] = {
        FND_DEVICE, FPGA_TEXT_LCD_DEVICE, SWITCH_DEVICE,
        FPGA_STEP_MOTOR_DEVICE, CONTROL_KEY_DEVICE, MMAP_DEVICE
    };

    /* open device files */
    int device_num;
    for(device_num = 0 ; device_num < MAX_DEVICE_NUMBER ; device_num++) {
        switch(device_num) {
            case FND:
                device_fds[device_num] = open(FND_DEVICE, O_RDWR);
                break;
            case LCD:
                device_fds[device_num] = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
                break;
            case SWITCH:
                device_fds[device_num] = open(SWITCH_DEVICE, O_RDWR);
                break;
            case MOTOR:
                device_fds[device_num] = open(FPGA_STEP_MOTOR_DEVICE, O_WRONLY);
                break;
            case CONTROL_KEY:
                device_fds[device_num] = open(CONTROL_KEY_DEVICE, O_RDONLY | O_NONBLOCK);
                break;
            case MMAP:
                device_fds[device_num] = open(MMAP_DEVICE, O_RDWR | O_SYNC);
                break;
        }
    }

    /* check if open failed */
    for(device_num = 0 ; device_num < MAX_DEVICE_NUMBER ; device_num++) {
        if (device_fds[device_num] == -1){
            printf("device open failed : %s\n", paths[device_num]);
            exit(0);
        }
    }

    /* init led with mmap */
    fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, device_fds[MMAP], FPGA_BASE_ADDRESS);
    led_addr = (unsigned char *)((void *)fpga_addr + LED_ADDR);
    return;
}

/* LED (mmap)*/
void write_led(int num) {
    *led_addr = LED_NUM(num);
    return;
}

void *pthread_toggle_led(void *arg) {
    bool is_key_input = (bool)arg;
    if (is_key_input) {
        while(true){
            write_led(3);
            usleep(1000000);
            write_led(4);
            usleep(1000000);
        }
    } else {
        while(true){
            write_led(7);
            usleep(1000000);
            write_led(8);
            usleep(1000000);
        }
    }

}

void toggle_led(bool is_key_input) {
    pthread_create((&toggle_tid),NULL,pthread_toggle_led,(void*)is_key_input);
    return;
}

void stop_toggle_led() {
    pthread_cancel(toggle_tid);
    return;
}


/* FND */
void write_fnd(unsigned char *buf) {
    unsigned char retval;
    unsigned char copy_buf[FND_MAX_DIGIT] = {0,};
    strcpy(copy_buf, buf);
    int i;
    for(i = 0 ; i < FND_MAX_DIGIT; i++) {
        copy_buf[0] -= FND_OFFSET;
    }
    if((retval = write(device_fds[FND], &copy_buf, 4)) < 0) {
        printf("fnd write error\n");
    }
    return;
}


/* LCD */
void write_lcd(unsigned char *buf) {
    unsigned char retval;
    if((retval = write(device_fds[LCD], buf, LCD_MAX_BUFF)) < 0) {
        printf("lcd write error\n");
    };
    return;
}

/* SWITCH */
void read_switch() {
    unsigned char retval;
    if((retval = read(device_fds[SWITCH], &sw_in_buf, sizeof(sw_in_buf))) < 0) {
        printf("switch read error\n");
    }
    return;
}

/* MOTOR */

/* CONTROL_KEY (NON_BLOCK) */
void read_control_key() {
    unsigned char retval;
    if((retval = read(device_fds[CONTROL_KEY], &ctl_buf, sizeof(ctl_buf))) < 0){
        printf("key read error\n");
    }
    return;
};