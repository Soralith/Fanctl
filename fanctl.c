#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#define VERSION "1.0"
#define EC_CMD  0x66
#define EC_DATA 0x62

enum { INDEX_CPU = 0x01, INDEX_GPU = 0x02 };

static int ec_wait() {
    int i = 30000;
    while ((inb(EC_CMD) & 2) && i-- > 0)
        usleep(50);
    return i > 0 ? 0 : -1;
}

static void ec_send(int cmd) { ec_wait(); outb(cmd, EC_CMD); }
static void ec_write(int data) { ec_wait(); outb(data, EC_DATA); }

static void set_fan(int index, int pct) {
    int duty = pct * 255 / 100;
    ec_send(0x99);
    ec_write(index);
    ec_write(duty);
    printf("%s fan set to %d%%\n", index == INDEX_CPU ? "CPU" : "GPU", pct);
}

static void set_auto(int index) {
    ec_send(0x99);
    ec_write(0xff);
    ec_write(index);
    printf("%s fan set to AUTO\n", index == INDEX_CPU ? "CPU" : "GPU");
}

static int ec_read_byte(int reg) {
    ec_wait(); outb(0x80, EC_CMD);
    ec_wait(); outb(reg, EC_DATA);
    ec_wait(); return inb(EC_DATA);
}

static void show_status() {
    int cpu_temp = ec_read_byte(0x07);
    int gpu_temp = ec_read_byte(0xCD);
    int raw_duty = ec_read_byte(0xCE);
    int rpm_hi   = ec_read_byte(0xD0);
    int rpm_lo   = ec_read_byte(0xD1);
    int duty_pct = raw_duty * 100 / 255;
    int rpm = rpm_hi > 0 ? 2156220 / ((rpm_hi << 8) + rpm_lo) : 0;

    printf("CPU: %dC  GPU: %dC\n", cpu_temp, gpu_temp);
    printf("Fan: %d%%  RPM: %d\n", duty_pct, rpm);
}

static void usage(const char *name) {
    fprintf(stderr,
        "Usage: %s [OPTIONS]\n"
        "  --cpu <1-100|auto>   Set CPU fan speed\n"
        "  --gpu <1-100|auto>   Set GPU fan speed\n"
        "  --status             Show current temps and fan info\n"
        "  --version            Show version\n"
        "  -h, --help           Show this help\n"
        "\n"
        "Examples:\n"
        "  sudo fanctl --cpu 50 --gpu 70\n"
        "  sudo fanctl --gpu auto\n"
        "  sudo fanctl --status\n",
        name);
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]); return 1; }

    int do_cpu = -1, do_gpu = -1;
    int cpu_auto = 0, gpu_auto = 0;
    int show = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--status") == 0) {
            show = 1;
        } else if (strcmp(argv[i], "--cpu") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "auto") == 0) {
                cpu_auto = 1;
            } else {
                do_cpu = atoi(argv[i]);
                if (do_cpu < 1 || do_cpu > 100) {
                    fprintf(stderr, "CPU speed must be 1-100 or 'auto'\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "--gpu") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "auto") == 0) {
                gpu_auto = 1;
            } else {
                do_gpu = atoi(argv[i]);
                if (do_gpu < 1 || do_gpu > 100) {
                    fprintf(stderr, "GPU speed must be 1-100 or 'auto'\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("fanctl " VERSION "\n");
            return 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (ioperm(EC_DATA, 1, 1) || ioperm(EC_CMD, 1, 1)) {
        perror("ioperm (run as root)");
        return 1;
    }

    if (cpu_auto)  set_auto(INDEX_CPU);
    if (do_cpu >= 0) set_fan(INDEX_CPU, do_cpu);

    if (gpu_auto)  set_auto(INDEX_GPU);
    if (do_gpu >= 0) set_fan(INDEX_GPU, do_gpu);

    if (show) {
        if (do_cpu >= 0 || cpu_auto || do_gpu >= 0 || gpu_auto)
            printf("---\n");
        show_status();
    }

    return 0;
}
