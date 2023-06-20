#include <stdio.h>
#include <stdlib.h>
#include "my_mmu.h"


char buffer_phymem[BUFFER_SIZE_PHYMEM]; //mem. fisica
char buffer_swapmem[BUFFER_SIZE_SWAPMEM]; //mem. swap


int main(int argc, char** argv) {
    
    if (!argv[1]){
        printf("nessun test selezionato\n");
        return 0;
    }

    int test_num=atoi(argv[1]);

    //inizializzo MMU
    printf("\nInizzializzo MMU\n");
    MMU* mmu=MMU_create(buffer_phymem,buffer_swapmem);
    MMU_print(mmu);
    
    LogicalAddress L_A;
    
    switch (test_num){

        case 1:
            printf("test selezionato: %d\n",test_num);

            //************ TEST 1 ************
            //Creo 10 processi, stampo MMU e la lista dei processi
        
            for (int i=1;i<11;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
                
            }

            MMU_print(mmu);
            MMU_process_update(mmu);
            List_print(mmu->MMU_processes);
        
            break;
        
        case 2:
            printf("test selezionato: %d\n",test_num);

            //************ TEST 2 ************
            //Creo 10 processi, richiedo 20 scritture e letture ad indirizzi diversi per ogni processo 
        
            for (int i=1;i<11;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
            }

            for (int i=0; i<201; ++i){
                
                L_A=FormLogAddr(i,i);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);
                
                char* read_byte=MMU_readByte(mmu,L_A);
                if(read_byte){
                    printf(": %c\n",*read_byte);
                }

                if(i%20==0){
                    MMU_process_update(mmu);
                }
            }
        
            break;

        case 3:
            printf("test selezionato: %d\n",test_num);
        
            //************ TEST 3 ************
            //Creo 10 processi, richiedo 20 scritture e letture ad indirizzi diversi per ogni processo, libero un processo e richiedo altre 20 scritture al processo corrente
        
            for (int i=1;i<11;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
            }

            for (int i=0; i<201; ++i){
                
                L_A=FormLogAddr(i%FRAME_INFO_SIZE,i%PENTRY_NUM);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);
                
                char* read_byte=MMU_readByte(mmu,L_A);
                if(read_byte){
                    printf(": %c\n",*read_byte);
                }

                if(i%20==0){
                    MMU_process_update(mmu);
                }
            }

            Process_release(mmu->MMU_processes->last->prev->prev,mmu);

            for (int i=0; i<70; ++i){
                
                L_A=FormLogAddr(i%FRAME_INFO_SIZE,i%PENTRY_NUM);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);

            }

            PageTable_print(mmu->curr_proc->pt);

            //NB! il processo non ha bisogno di richiedere swap poichè la memoria fisica ha 255 posti all'inizio 20 sono occupati da PageTables e 200 da frame dei processi 
            //liberandone uno si liberano altri 22 posti ->198 posti occupati, poichè infine il processo corrente è il primo processo quindi 20 delle sue scritture 
            //sulla prime 70 entry saranno non avranno page fault ne segue che le restanti 50 scritture richiederanno frame (57 disponibili) 


            break;

        case 4:
            printf("test selezionato: %d\n",test_num);
        
            //************ TEST 4 ************
            //Creo 3 processi, richiedo 70 scritture e letture ad indirizzi diversi per ogni processo, richiedo altre 200 scritture al processo corrente
        
            for (int i=1;i<4;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
            }

            for (int i=0; i<211; ++i){
                
                L_A=FormLogAddr(i%FRAME_INFO_SIZE,i%PENTRY_NUM);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);
                
                char* read_byte=MMU_readByte(mmu,L_A);
                if(read_byte){
                    printf(": %c\n",*read_byte);
                }

                if(i%70==0){
                    MMU_process_update(mmu);
                }
            }

            for (int i=0; i<200; ++i){
                
                L_A=FormLogAddr(i%FRAME_INFO_SIZE,i%PENTRY_NUM);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);

            }

            PageTable_print(mmu->curr_proc->pt);

            break;

        case 5:
            printf("test selezionato: %d\n",test_num);
        
            //************ TEST 5 ************
            //Creo 1 processo,richiedo altre 4500 scritture (cicliche) al processo corrente
            
            for (int i=1;i<2;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
            }
            
            for (int i=0; i<4500; ++i){
                
                L_A=FormLogAddr(i%FRAME_INFO_SIZE,i);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);

            }

            break;

        case 6:
            printf("test selezionato: %d\n",test_num);
        
            //************ TEST 6 ************
            //Creo 50 processi, richiedo 4 scritture e letture ad indirizzi diversi per ogni processo, richiedo altre 100 scritture al processo corrente
        
            for (int i=1;i<50;i++){
                Process *item=Process_alloc();
                Process_init(item,i,mmu);
            }

            for (int i=0; i<201; ++i){
                
                L_A=FormLogAddr(i,i);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);
                
                if(i%2==0){
                char* read_byte=MMU_readByte(mmu,L_A);
                    if(read_byte){
                    printf(": %c\n",*read_byte);
                    }
                }
                
                if(i%4==0){
                    MMU_process_update(mmu);
                }
            }
            for (int i=0; i<100; ++i){
                L_A=FormLogAddr((i+1200)%FRAME_INFO_SIZE,(i+1200)%PENTRY_NUM);
                char c=i%FRAME_NUM;

                MMU_writeByte(mmu,L_A,c);
            }

            PageTable_print(mmu->curr_proc->pt);


            break;
    
        default:
            printf("test selezionato non esiste\n");
            break;
    }


    return 0;
}