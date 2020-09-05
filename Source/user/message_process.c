#include "message_process.h"


PNode in_head;
PNode out_head;
char **in_compatible;
char **out_compatible;

void head_init()
{
    in_head=(PNode)calloc(1,sizeof(struct NODE));
    out_head=(PNode)calloc(1,sizeof(struct NODE));
    in_compatible=(char **)calloc(1,sizeof(char*));
    out_compatible=(char **)calloc(1,sizeof(char*));
}

void head_free()
{
    free(in_head);
    free(out_head);
}


void message_list_init()            //init in & out list
{
    int i=0;
    PNode p1,p2;

    p1=in_head->link;
    in_head->link=NULL;
    in_head->drv_type=0;
    
    while(p1!=NULL)
    {
        p2=p1;
        p1=p1->link;
        free(p2);
    }

    p1=out_head->link;
    out_head->link=NULL;
    out_head->drv_type=0;

    while(p1!=NULL)
    {
        p2=p1;
        p1=p1->link;
        free(p2);
    }   

    while(*(in_compatible+i)!=NULL)
    {
        free(*(in_compatible+i));
        i++;
    }
    free(in_compatible);
}

void message_list_add(char *name,enum message_process_state sta,enum plat_driver_type type,int analog_quantity)
{
    PNode pnew=(PNode)malloc(sizeof(struct NODE));

    pnew->link=NULL;
    strcpy(pnew->compatible,name);
    if(type==drv_trigger_high)
    {
        strcat(pnew->compatible,"-TH");
    }else if (type==drv_trigger_low)
    {
        strcat(pnew->compatible,"-TL");
    }else if (type==drv_analog)
    {
        strcat(pnew->compatible,"-A");
    }
    pnew->drv_type=type;
    pnew->analog_quantity=analog_quantity;

    if(sta==condition_insert)
    {
        pnew->link=in_head->link;
        in_head->link=pnew;

        *(in_compatible+in_head->drv_type)=(char *)malloc(20*sizeof(char));
        strcpy(*(in_compatible+in_head->drv_type),name);
        /*
        *(in_compatible+in_head->drv_type)=name;
        */
        if(type==drv_trigger_high)
        {
            strcat(*(in_compatible+in_head->drv_type),"-TH");
        }else if (type==drv_trigger_low)
        {
            strcat(*(in_compatible+in_head->drv_type),"-TL");
        }else if (type==drv_analog)
        {
            strcat(*(in_compatible+in_head->drv_type),"-A");
        }
        in_head->drv_type++;               //the num of in_node++ 
        
    }else if(sta==result_insert)
    {
        pnew->link=out_head->link;
        out_head->link=pnew;

        *(out_compatible+out_head->drv_type)=(char *)malloc(20*sizeof(char));
        strcpy(*(out_compatible+out_head->drv_type),name);
        /*
        *(out_compatible+out_head->drv_type)=name;
        */ 
        if(type==drv_trigger_high)
        {
            strcat(*(out_compatible+out_head->drv_type),"-TH");
        }else if (type==drv_trigger_low)
        {
            strcat(*(out_compatible+out_head->drv_type),"-TL");
        }else if (type==drv_analog)
        {
            strcat(*(out_compatible+out_head->drv_type),"-A");
        }	    
        out_head->drv_type++;               //the num of out_node++ 
    }else if(sta==init_insert)
    {
        ;                       //insert init list
    }
}

uint8_t individual_process(char *string,enum message_process_state sta,uint8_t j,uint8_t string_len)
{
    uint8_t compatible_len;
    int analog_num=0,analog_direc=-1;
    char compatible[20];
    enum plat_driver_type type;
    compatible_len=0;
    
    while(j<string_len)
    {
        while(string[j]!='\'')
        {
            compatible[compatible_len++]=string[j++];
            if(compatible_len>20)
            {
                printf(" Format error:the name is too long ");
            }
        }
	compatible[compatible_len]=0;
        j++;
        if(compatible_len!=0)
        {
            //H L A PROCESS
            if(string[j]=='H')
            {
                type=drv_trigger_high;
                j++;
                message_list_add(compatible,sta,type,0);
                return j;
            }else if (string[j]=='L')
            {
                type=drv_trigger_low;
                j++;
                message_list_add(compatible,sta,type,0);
                return j;                
            }else if (string[j]=='A')  
            {
                type=drv_analog;
                j++;
                if(sta==condition_insert)                       //# only appear in w condition 
                {
                    if(string[j]=='#')
                    {
                        j++;
                        while(isdigit(string[j]))
                        {
                            analog_num=analog_num*10+string[j]-'0';
                            j++;
                        }
                        message_list_add(compatible,sta,type,analog_num);
                        return j;
                    }else
                    {
                        while(isdigit(string[j]))
                        {
                            analog_num=analog_num*10+string[j]-'0';
                            j++;
                        }
                        analog_num*=analog_direc;
                        message_list_add(compatible,sta,type,analog_num);
                        j++;
                        return j;                       
                    }
                }else
                {
                    while(isdigit(string[j]))
                    {
                        analog_num=analog_num*10+string[j]-'0';
                    }
                    message_list_add(compatible,sta,type,analog_num);
                    return j;
                }
            }
        }else
        {
            j++;
        }
    }
}

uint8_t content_process(char *string,enum message_process_state sta,uint8_t j,uint8_t string_len)
{
    char end_mark;
    int i;
    // set end_mark
    if(sta==condition_insert)
    {
        end_mark=',';
    }else 
    {
        end_mark='.';
    }
    //content condition process
    while(string[j]!=end_mark&&j<string_len)
    {
        //get compatible 
        if(string[j]=='\'')
        {
            j++;
            j=individual_process(string,sta,j,string_len);
        }else
        {
            j++;
        }

    }
    return j;
}

char string_process(const char *string)
{
    uint8_t i=0,z=0,string_len=strlen(string),head_mark=0;
    enum message_process_state sta;
    char flag=0;
    PNode p;
	
    if(!head_mark)
    {
        head_init();
    }

//    message_list_init();

    in_compatible=(char **)calloc(10, sizeof(char*));
    out_compatible=(char **)calloc(10, sizeof(char*));
    
    for(i=0;0<=i&&i<string_len;i++)
    {
        
        if(string[i]=='W')
        {
            flag=1;
            sta=condition_insert;
            i=content_process(string,sta,i,string_len);
        }else if (string[i]=='S')
        {
            flag=1;
            if(sta==condition_insert)
            {
                sta=result_insert;
            }else
            {
                sta=init_insert;
            }
            i=content_process(string,sta,i,string_len);
        }else if (string[0]=='O')
        {
            flag=0;
        }
    }


    
    if(i==string_len)               //正常处理完
    {
        if(sta!=init_insert)
        {
            p=in_head->link;
            while(p!=NULL)
            {
                z=0;
                platform_bus_probe(p->compatible, drv_in, p->drv_type, NULL, out_head->drv_type, out_compatible, p->analog_quantity);
                p=p->link;
            }
            
            p=out_head->link;
            while(p!=NULL)
            {
                z=0;
                platform_bus_probe(p->compatible,drv_out,p->drv_type, NULL,in_head->drv_type,in_compatible, p->analog_quantity);
                p=p->link;
            }

        }else
        {
            ;//init list
        }

    }
    message_list_init();
    head_free();

    return flag;
}

