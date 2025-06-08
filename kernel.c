#include <stdint.h>
#include <stddef.h>

/* Basic I/O */
static inline uint8_t inb(uint16_t p){uint8_t r;__asm__("inb %1,%0":"=a"(r):"Nd"(p));return r;}
static inline void outb(uint16_t p,uint8_t v){__asm__("outb %0,%1"::"a"(v),"Nd"(p));}

/* Console */
static volatile uint16_t* const VIDEO=(volatile uint16_t*)0xB8000;
static size_t cursor=0;
static void put_char(char c){if(c=='\n')cursor=(cursor/80+1)*80;else if(c=='\b'){if(cursor){cursor--;VIDEO[cursor]=' '|0x0F00;}}else VIDEO[cursor++]=(uint16_t)c|0x0F00;if(cursor>=80*25)cursor=0;}
static void print(const char*s){while(*s)put_char(*s++);} 
static void print_dec(int n){char b[16];int i=0;if(n==0){put_char('0');return;}while(n>0&&i<16){b[i++]='0'+n%10;n/=10;}while(i--)put_char(b[i]);}

/* IDT */
struct idt_entry{uint16_t off_low;uint16_t sel;uint8_t zero;uint8_t flags;uint16_t off_high;}__attribute__((packed));
struct idt_ptr{uint16_t limit;uint32_t base;}__attribute__((packed));
static struct idt_entry idt[256];
static struct idt_ptr idtp;
extern void irq0_stub(void);extern void irq1_stub(void);extern void first_task_switch(uint32_t);
static void idt_set_gate(int n,void*h){uint32_t a=(uint32_t)h;idt[n].off_low=a&0xFFFF;idt[n].sel=0x08;idt[n].zero=0;idt[n].flags=0x8E;idt[n].off_high=a>>16;}
static void idt_install(void){idtp.limit=sizeof(idt)-1;idtp.base=(uint32_t)idt;__asm__("lidt (%0)"::"r"(&idtp));}

/* PIC and timer */
static void pic_remap(void){outb(0x20,0x11);outb(0xA0,0x11);outb(0x21,0x20);outb(0xA1,0x28);outb(0x21,0x04);outb(0xA1,0x02);outb(0x21,0x01);outb(0xA1,0x01);outb(0x21,0);outb(0xA1,0);} 
static void pit_init(void){uint32_t d=1193180/100;outb(0x43,0x36);outb(0x40,d&0xFF);outb(0x40,d>>8);} 

/* Queues and tasks */
#define QSIZE 64
struct queue{char b[QSIZE];int h,t;};
static int q_empty(struct queue*q){return q->h==q->t;}
static void q_push(struct queue*q,char c){if(((q->t+1)%QSIZE)!=q->h){q->b[q->t]=c;q->t=(q->t+1)%QSIZE;}}
static int q_pop(struct queue*q,char*c){if(q_empty(q))return 0;*c=q->b[q->h];q->h=(q->h+1)%QSIZE;return 1;}

#define STACK_SIZE 4096
struct task{uint32_t esp;uint32_t stack[STACK_SIZE/4];struct queue in;char line[128];size_t pos;};
static struct task tasks[2];
static int focus=0;static int current=0;

/* Forward task code */
static void echo_task(void);static void wc_task(void);
static void task_init(struct task*t,void(*e)(void)){uint32_t*sp=t->stack+STACK_SIZE/4;*(--sp)=0x202;*(--sp)=0x08;*(--sp)=(uint32_t)e;for(int i=0;i<8;i++)*(--sp)=0;t->esp=(uint32_t)sp;t->in.h=t->in.t=0;t->pos=0;}

/* Scheduler */
static uint32_t schedule(uint32_t esp){tasks[current].esp=esp;current=(current+1)%2;return tasks[current].esp;}
uint32_t timer_interrupt(uint32_t esp){uint32_t n=schedule(esp);outb(0x20,0x20);return n;}
uint32_t keyboard_interrupt(uint32_t esp){uint8_t sc=inb(0x60);outb(0x20,0x20);if(!(sc&0x80)){char c=0;if(sc==0x1C)c='\n';else if(sc==0x0E)c='\b';else if(sc==0x0F)c='\t';else{static const char map[128]={[0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',[0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',[0x15]='y',[0x16]='u',[0x17]='i',[0x18]='o',[0x19]='p',[0x1E]='a',[0x1F]='s',[0x20]='d',[0x21]='f',[0x22]='g',[0x23]='h',[0x24]='j',[0x25]='k',[0x26]='l',[0x2C]='z',[0x2D]='x',[0x2E]='c',[0x2F]='v',[0x30]='b',[0x31]='n',[0x32]='m',[0x39]=' ',};c=map[sc];}if(c){if(c=='\t'){focus=(focus+1)%2;print("\n");if(focus==0)print("Echo> ");else print("WordCount> ");}else{q_push(&tasks[focus].in,c);}}}return esp;}

/* Task code */
static void echo_task(void){print("Echo> ");for(;;){char c;if(q_pop(&tasks[0].in,&c)){if(c=='\n'){tasks[0].line[tasks[0].pos]='\0';print("\nYou typed: ");print(tasks[0].line);print("\nEcho> ");tasks[0].pos=0;}else if(c=='\b'){if(tasks[0].pos){tasks[0].pos--;print("\b");}}else{if(tasks[0].pos<sizeof(tasks[0].line)-1){tasks[0].line[tasks[0].pos++]=c;print((char[]){c,0});}}}for(volatile int i=0;i<100000;i++);}}
static int count_words(const char*s){int c=0,in=0;for(size_t i=0;s[i];i++){if(s[i]==' ')in=0;else if(!in){in=1;c++;}}return c;}
static void wc_task(void){print("WordCount> ");for(;;){char c;if(q_pop(&tasks[1].in,&c)){if(c=='\n'){tasks[1].line[tasks[1].pos]='\0';int w=count_words(tasks[1].line);print("\nWord count: ");print_dec(w);print("\nWordCount> ");tasks[1].pos=0;}else if(c=='\b'){if(tasks[1].pos){tasks[1].pos--;print("\b");}}else{if(tasks[1].pos<sizeof(tasks[1].line)-1){tasks[1].line[tasks[1].pos++]=c;print((char[]){c,0});}}}for(volatile int i=0;i<100000;i++);}}

/* Kernel entry */
void kernel_main(void){print("SimpleKernel Preemptive Demo\n");pic_remap();idt_set_gate(32,irq0_stub);idt_set_gate(33,irq1_stub);idt_install();pit_init();task_init(&tasks[0],echo_task);task_init(&tasks[1],wc_task);focus=0;current=0;first_task_switch(tasks[0].esp);while(1);} 
