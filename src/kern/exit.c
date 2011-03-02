#include <param.h>
#include <x86.h>
#include <proto.h>
#include <proc.h>

#include <page.h>
#include <vm.h>

#include <conf.h>                      

/* exit.c - 2011 fleurer
 * */

/* terminate the currenct proccess into ZOMBIE, and tell its parent.
 * the struct proc is freed by each proc's parent.
 * */
int do_exit(int ret){
    struct file *fp;
    struct proc *p;
    uint fd, nr;

    cu->p_ret = ret;
    // make this process Zombie, give all the children to proc[1] 
    // and tell its parent
    cu->p_stat = SZOMB;
    cu->p_chan = 0;
    for (nr=0; nr<NPROC; nr++) {
        if ((p=proc[nr]) && (p->p_ppid==cu->p_pid)) {
            p->p_ppid = 1;
        }
    }
    // clear all the signal handlers
    for (nr=0; nr<NSIG; nr++) {
        cu->p_sigact[nr].sa_handler = SIG_DFL;
        cu->p_sigact[nr].sa_flags = 0;
    }
    // close all the opened files, and iput the directories.
    for (fd=0; fd<NOFILE; fd++){
        fp = cu->p_ofile[fd];
        if (fp != NULL) {
            do_close(fd);
        }
    }
    iput(cu->p_wdir);
    iput(cu->p_iroot);
    // free the address space
    vm_clear(&cu->p_vm);
    kfree(cu->p_vm.vm_pgd, PAGE);
    // tell its parent
    sigsend(cu->p_ppid, SIGCHLD, 1);
    //
    swtch();
    return 0;
}
