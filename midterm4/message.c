/******** message.c file ************/
#include "type.h"
// READ Chapter 5.13.4.3; Use its code

#define NMBUF 10
struct semaphore nmbuf, mlock;
MBUF mbuf[NMBUF], *mbufList;

int menqueue(MBUF **queue, MBUF *p)
{
    // enter p into queue by priority
    MBUF *q = *queue;
    if (q == 0)
    {
        *queue = p;
        p->next = 0;
        return 0;
    }
    if (p->pri > q->pri)
    {
        *queue = p;
        p->next = q;
        return 0;
    }
    
    // insert before first encountered node with lower priority
    while (q->next != 0 && p->pri <= q->next->pri)
    {
        q = q->next;
    }
    p->next = q->next;
    q->next = p;
}

MBUF *mdequeue(MBUF **queue)
{
    // return first queue element
    MBUF *q = *queue;
    if (q == 0)
        return 0;
    // pop
    *queue = q->next;
    return q;

}
    
		    
int msg_init()
{
    int i; MBUF *mp;
    printf("msg_init()\n");
    mbufList = 0;
    for (i = 0; i < NMBUF; i++)
    {
        menqueue(&mbufList, &mbuf[i]);
    }
    nmbuf.value = NMBUF; nmbuf.queue = 0;
    mlock.value = 1; mlock.queue = 0;
}

MBUF *get_mbuf()
{
    P(&nmbuf);
    P(&mlock);
    MBUF *mp = mdequeue(&mbufList);
    V(&mlock);
    return mp;
}

int put_mbuf(MBUF *mp)
{
    P(&mlock);
    menqueue(&mbufList, mp);
    V(&mlock);
    V(&nmbuf);
}

int send(char *msg, int pid)
{
    printf("SENDING...\n");
    PROC *p = &proc[pid];
    MBUF *mp = get_mbuf();
    
    mp->pid = running->pid;
    mp->pri = 1;

    strcpy(mp->contents, msg);
    printf("msg=%s\n", msg);

    P(&p->mlock);
    menqueue(&p->mqueue, mp);
    V(&p->mlock);
    V(&p->nmsg);
    printf("SENT.\n");
    return 0;
}

int recv(char *msg)
{
    printf("RECEIVING...\n");
    P(&running->nmsg);
    P(&running->mlock);

    MBUF *mp = mdequeue(&running->mqueue);
    V(&running->mlock);
    strcpy(msg, mp->contents);
    int senderPID = mp->pid;
    put_mbuf(mp);
    return senderPID;
}


  
