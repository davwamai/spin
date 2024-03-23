#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_PORT 1001

int interrupted = 0;

void signal_handler(int sig)
{
  interrupted = 1;
}

void neoncopy(void *dst, volatile void *src, int cnt)
{
  asm volatile
  (
    "loop_%=:\n"
    "vldm %[src]!, {q0, q1, q2, q3}\n"
    "vstm %[dst]!, {q0, q1, q2, q3}\n"
    "subs %[cnt], %[cnt], #64\n"
    "bgt loop_%="
    : [dst] "+r" (dst), [src] "+r" (src), [cnt] "+r" (cnt)
    :
    : "q0", "q1", "q2", "q3", "cc", "memory"
  );
}

/*  f_out: Desired output frequency.
    f_clk: System clock frequency, in Hz.
    B_theta: Number of bits in the phase accumulator.*/
int phase_increment(double f_out, double f_clk, int B_theta) {
	  double phase_increment = ((10E5*f_out) * (pow(2,B_theta))) / f_clk;
    return (int)phase_increment;
}

int main ()
{

  printf("--- Signal Processing Instrument for NMR ---\n");
  int mmapfd, sockServer, sockClient;
  int position, limit, offset, _inc, user_delay, delay;
  double user_inc;
  volatile uint32_t *slcr, *axi_hp0;
  volatile void *cfg, *sts, *ram;
  void *buf;
  cpu_set_t mask;
  struct sched_param param;
  struct sockaddr_in addr;
  int yes = 1;

  memset(&param, 0, sizeof(param));
  param.sched_priority = sched_get_priority_max(SCHED_FIFO);
  sched_setscheduler(0, SCHED_FIFO, &param);

  CPU_ZERO(&mask);
  CPU_SET(1, &mask);
  sched_setaffinity(0, sizeof(cpu_set_t), &mask);

  printf("opening /dev/mem\n");

  if((mmapfd = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    return 1;
  }

  printf("sysconf(_SC_PAGESIZE) = %d\n", sysconf(_SC_PAGESIZE));


  slcr = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmapfd, 0xF8000000);
  axi_hp0 = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmapfd, 0xF8008000);
  sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmapfd, 0x40001000);
  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmapfd, 0x40000000);
  ram = mmap(NULL, 128*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, mmapfd, 0x1E000000);
  buf = mmap(NULL, 64*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

  printf("good mmap\n");

  /* set HP0 bus width to 64 bits */
  slcr[2] = 0xDF0D;
  slcr[144] = 0;
  axi_hp0[0] &= ~1;
  axi_hp0[5] &= ~1;

  printf("slcr = %p\n", slcr);
  printf("axi_hp0 = %p\n", axi_hp0);

  if((sockServer = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket");
    return 1;
  }

  setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));

  printf("setting up listening address\n");
  /* setup listening address */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(TCP_PORT);

  if(bind(sockServer, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    return 1;
  }
  printf("good bind\n");

  listen(sockServer, 1024);

  printf("waiting for connection\n");

  while(!interrupted)
  {
    // /* enter reset mode */
    *(uint8_t *)(cfg + 0) &= ~1;
    usleep(100);
    *(uint8_t *)(cfg + 0) &= ~2;
    usleep(100);

    /* reset dac */
    *(uint32_t *)(cfg + 8) = 0;


    if((sockClient = accept(sockServer, NULL, NULL)) < 0)
    {
      perror("accept");
      return 1;
    }

    printf("connected. IRQ\n");

    limit = 32*1024;

    signal(SIGINT, signal_handler);

    printf("Enter Frequency in MHz: \n");
    scanf("%le", &user_inc);

    _inc = phase_increment(user_inc, 125000000.0, 32);
		printf("Calculated %08x phase increment for frequency %d \r\n", _inc, user_inc);

    printf("Enter CPMG delay in clock cycles (125MHz): \n");
    scanf("%d", &user_delay);

    printf("Delay set to: %d cycles\n", user_delay);

    delay = user_delay;

    *(uint8_t *)(cfg + 12) = delay;
    /* enter normal operating mode */
    *(uint8_t *)(cfg + 0) |= 3;
    /* write inc to dac c1 */
    *(uint32_t *)(cfg + 4) = _inc;

    while(!interrupted)
    {
      *(uint8_t *)(cfg + 8) = 1;

      /* read ram writer position */
      position = *(uint32_t *)(sts + 12);

      /* send 256 kB if ready, otherwise sleep 0.1 ms */
      if((limit > 0 && position > limit) || (limit == 0 && position < 32*1024))
      {
        offset = limit > 0 ? 0 : 256*1024;
        limit = limit > 0 ? 0 : 32*1024;
        neoncopy(buf, ram + offset, 256*1024);
        if(send(sockClient, buf, 256*1024, MSG_NOSIGNAL) < 0) break;
      }
      else
      {
        usleep(100);
      }
    }

    signal(SIGINT, SIG_DFL);
    close(sockClient);
  }

  /* enter reset mode */
  *(uint8_t *)(cfg + 0) &= ~1;
  usleep(100);
  *(uint8_t *)(cfg + 0) &= ~2;

  close(sockServer);

  return 0;
}
