/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "../type.h"

int goUmode();

int exec(char *uline)
{
  int i, *ip;
  char *cp, kline[64]; 
  PROC *p = running;
  char file[32], filename[32];
  int *usp, *ustacktop;
  u32 BA, Btop, Busp, uLINE;
  char line[32];

  //printf("EXEC: proc %d uline=%x ", running->pid, uline);

  // line is in Umode image at p->pgdir[2048]&0xFFF00000=>can access from Kmode
  // char *uimage = (char *)(p->pgdir[2048] & 0xFFF00000);
  BA = (p->res->pgdir[2048] & 0xFFFFF000);
  Btop = BA + 0x100000;  // top of 1MB Uimage
  //printf("EXEC: proc %d Uimage at %x\n", running->pid, BA);
 
  uLINE = BA + ((int)uline - 0x80000000);
  kstrcpy(kline, (char *)uLINE);
  // NOTE: may use uline directly 

  //printf("EXEC: proc %d line = %s   ", running->pid, kline); 

  // first token of kline = filename
  cp = kline; i=0;
  while(*cp != ' ' && *cp){
    filename[i] = *cp;
    i++; cp++;
  } 
  filename[i] = 0;
  /*
  kstrcpy(file, "/bin/");
  kstrcat(file, filename);
  */
  BA = p->res->pgdir[2048] & 0xFFFFF000;
  //kprintf("load file %s to %x\n", file, BA);

  // load filename to Umode image 
  if (load(filename, p) <= 0 ){
    printf("exec loading error\n");
    return -1;
  }
  //  printf("after loading ");

  // copy kline to high end of Ustack in Umode image
  Btop = BA + 0x100000;
  Busp = Btop - 32;

  cp = (char *)Busp;
  kstrcpy(cp, kline);
  //printf("cp=%x contents=%s\n", cp, cp);

  p->usp = (int *)VA(0x100000 - 32);
  p->upc = (int *)VA();
  
  p->kstack[SSIZE-14] = (int)VA(0x100000 - 32); // R0 to Umode
  p->kstack[SSIZE-1] = (int)VA(0);              // ulr to Umode
  //printf("usp=%x contents=%s\n", p->usp, (char *)p->usp);
  strcpy(running->res->name, filename);

  //kprintf("kexec exit\n");
  return p->usp;       // return value may ovewrite saved R0 in kstack
}

int kfe(char *cmdline)
{
  int i, j;
  char *cp, *cq;
  u32 *ucp, *upp;
  int *ip;
  char kline[64], file[32], filename[32], line[32];
  int *usp, *ustacktop;
  u32 BA, Btop, Busp, uLINE;

  PROC *p = getproc();
  if (p == 0)
  {
    kprintf("fork failed\n");
    return -1;
  }

  p->status = READY;
  p->ppid = running->pid;
  p->parent = running;
  p->priority = 1;
  p->inkmode = 1;
  p->time = 0;
  p->cpu = 0;
  p->type = PROCESS;
  p->cpsr = 0x10;

  p->res->size = running->res->size;
  p->res->uid = running->res->uid;
  p->res->gid = running->res->gid;
  p->res->cwd = running->res->cwd;
  p->tcount = 1;
  p->res->cwd->refCount++;
  strcpy(p->res->tty, running->res->tty);

  p->res->signal = 0;
  for (i = 0; i < NSIG; i++)
    p->res->sig[i] = 0;

  p->res->mqueue = 0;
  p->res->mlock.value = 1;
  p->res->mlock.queue = 0;
  p->res->message.value = 0;
  p->res->message.queue = 0;

  BA = (p->res->pgdir[2048] & 0xFFF00000);

  cp = cmdline;
  ucp = (u32 *)BA;
  while (*cp)
  {
    *ucp = *cp;
    ucp++;
    cp++;
  }

  for (i = 1; i <= 14; i++)
  {
    p->kstack[SSIZE - i] = running->kstack[SSIZE - i];
  }

  p->kstack[SSIZE - 14] = 0;

  p->kstack[SSIZE - 1] = (int)(running->upc);
  p->kstack[SSIZE - 15] = (int)goUmode;
  p->ksp = &(p->kstack[SSIZE - 28]);

  for (i = 0; i < NFD; i++)
  {
    p->res->fd[i] = running->res->fd[i];
    if (p->res->fd[i] != 0)
    {
      p->res->fd[i]->refCount++;
      if (p->res->fd[i]->mode == READ_PIPE)
      {
        p->res->fd[i]->pipe_ptr->nreader++;
      }
      if (p->res->fd[i]->mode == WRITE_PIPE)
      {
        p->res->fd[i]->pipe_ptr->nwriter++;
      }
    }
  }

  BA = (p->res->pgdir[2048] & 0xFFFFF000);
  Btop = BA + 0x100000; // top of 1MB Uimage

  // uLINE = BA + ((int)cmdline - 0x80000000);
  // kstrcpy(kline, (char *)uLINE);
  kstrcpy(kline, cmdline);

  cp = kline;
  i = 0;
  while (*cp != ' ' && *cp)
  {
    filename[i] = *cp;
    i++;
    cp++;
  }
  filename[i] = 0;

  BA = p->res->pgdir[2048] & 0xFFFFF000;

  if (load(filename, p) <= 0)
  {
    printf("exec loading error\n");
    return -1;
  }

  Btop = BA + 0x100000;
  Busp = Btop - 32;

  cp = (char *)Busp;
  kstrcpy(cp, kline);

  p->usp = (int *)VA(0x100000 - 32);
  p->upc = (int *)VA();

  p->kstack[SSIZE - 14] = (int)VA(0x100000 - 32); // R0 to Umode
  p->kstack[SSIZE - 1] = (int)VA(0);              // ulr to Umode
  strcpy(running->res->name, filename);

  enqueue(&readyQueue, p);
  printQ(readyQueue);

  return p->pid;
}