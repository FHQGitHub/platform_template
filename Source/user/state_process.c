#include "state_process.h"

char si4463_send_data[4];

// void get_addr()
// {
    
// }

void position_change(int order,int sta)
{
    if(order/8==0)
    {
        if((si4463_send_data[1]&(1<<order))!=sta)
        {
            if(sta)
            {
                si4463_send_data[1]|=1<<order;
            }else
            {
                si4463_send_data[1]&=~(1<<order);
            }
        }    
    }else if (order/8==1)
    {
        order%=8;
        if((si4463_send_data[2]&(1<<order))!=sta)
        {
            if(sta)
            {
                si4463_send_data[2]|=1<<order;
            }else
            {
                si4463_send_data[2]&=~(1<<order);
            }
        }  
    }else if (order/8==2)
    {
        order%=8;
        if((si4463_send_data[3]&(1<<order))!=sta)
        {
            if(sta)
            {
                si4463_send_data[3]|=1<<order;
            }else
            {
                si4463_send_data[3]&=~(1<<order);
            }
        }  
    }
    
}

// void position_init()
// {

// }
