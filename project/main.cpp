#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <tchar.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
const wchar_t CLASS_NAME[] = L"Sample Window class";
const int sizeX = 750;
const int sizeY = 500;
bool result, playing, beginning;
void assign_rect(RECT *rect, int x1, int y1, int x2, int y2);

struct PIXEL
{
    RECT rect;
    HBRUSH color;
    PIXEL& operator=(const PIXEL &o){
        if(this!=&o){
            rect = o.rect;
            color = o.color;
        }
        return *this;
    }
};

void assign_pixel(PIXEL *pixel, int left, int top, int right, int bottom, HBRUSH hbrush);
PIXEL board[100][100];

struct LASER
{
    PIXEL part;
    int x,y,mul_x,cd,rate;
    double m,i;
    LASER(){
        assign_pixel(&part, 1000, 1000, 1001, 1001, CreateSolidBrush(RGB(255, 0, 0)));
        x=y=mul_x=cd=0;
        rate=1;
    }

    LASER& operator=(const LASER &o){
        if (this!=&o){
            part=o.part;
            rate = o.rate;
            x = o.x, y = o.y, mul_x = o.mul_x;
            cd = o.cd;
        }
        return *this;
    }

    bool Create(int x1, int y1, int x2, int y2){
        if(cd) return false;
        cd = 1, i = 1;
        int dx, dy;
        x = x1, y = y1;
        dx = x2-x1, dy = y2-y1;
        if(dx) m = dy/(double)dx;
        else mul_x = 0, m = 1;
        if(dx>0) mul_x = 1;
        else if(dx<0) mul_x = -1;
        if(dy>0 && m<0) m*=-1;
        else if(dy<0 && m>0) m*=-1;
        assign_rect(&part.rect, x, y, x, y);
        return true;
    }

    void Show(HDC hdc){
        FillRect(hdc, &part.rect, part.color);
        return;
    }

    void Destroy(HWND hwnd){
        cd = 0;
        part.rect = {1000, 1000, 1001, 1001};
        InvalidateRect(hwnd, &part.rect, 1);
    }

    void ChangePos(HWND hwnd){
        if(!cd) return;
        InvalidateRect(hwnd, &part.rect, 1);
        i+=rate;
        int new_x = (int)round(x+i*mul_x), new_y = (int)round(y+i*m);
        if(new_x<0 || new_y<0 || new_x>=sizeX/10 || new_y>=sizeY/10){
            Destroy(hwnd);
            return;
        }
        assign_rect(&part.rect, new_x, new_y, new_x, new_y);
        InvalidateRect(hwnd, &part.rect, 1);
        return;
    }

    bool Overlap(RECT rect, HWND hwnd){
        if(!cd) return 0;
        if(part.rect.left>=rect.left && part.rect.top>=rect.top && part.rect.right<=rect.right && part.rect.bottom<=rect.bottom){
            Destroy(hwnd);
            return 1;
        }
        return 0;
    }

};

struct PLAYER
{
    PIXEL part[2],hp_bar,bar_cover;
    RECT parry_status, tel_status;
    int left,top,laser_cnt,hp,atk,walkSpeed;
    int parry_cd,parry_dur,parry_MAXDUR,parry_MAXCD;
    bool parrying;
    LASER laser[50],parry_laser[50];
    RECT hitbox;
    PLAYER(){
        left=sizeX/10-1, top=sizeY/10-3;
        laser_cnt = 3, hp = 100, atk = 10, walkSpeed = 1, parry_MAXDUR = 5, parry_MAXCD = 20, parry_cd = parry_MAXCD;
        assign_pixel(&part[0], sizeX-10, sizeY-30, sizeX, sizeY-20, CreateSolidBrush(RGB(228, 212, 199)));
        assign_pixel(&part[1], sizeX-10, sizeY-20, sizeX, sizeY, CreateSolidBrush(RGB(220, 5, 50)));
        assign_pixel(&hp_bar, 1, 1, 101, 10, CreateSolidBrush(RGB(0, 153, 51)));
        assign_pixel(&bar_cover, 0, 0, 102, 11, CreateSolidBrush(RGB(255, 255, 255)));
        parry_status = {110, 1, 200, 20};
        hitbox = {sizeX-10, sizeY-30, sizeX, sizeY};
        for(int i=0;i<50;i++){
            parry_laser[i].part.color = CreateSolidBrush(RGB(0, 255, 255));
            parry_laser[i].rate = 2;
        }
    }

    PLAYER& operator=(const PLAYER &o){
        if (this!=&o){
            part[0] = o.part[0], part[1] = o.part[1];
            hp_bar = o.hp_bar, bar_cover = o.bar_cover, parry_status = o.parry_status;
            left = o.left, top = o.top;
            laser_cnt = o.laser_cnt, hp = o.hp;
            parry_cd = o.parry_cd, parry_dur = o.parry_dur, parrying = o.parrying;
            for(int i=0;i<50;i++){
                laser[i] = o.laser[i];
                parry_laser[i] = o.parry_laser[i];
            }
            hitbox = o.hitbox;
        }
        return *this;
    }

     bool Overlap(RECT rect, int ci, int cj, HWND hwnd){
        if(cj*10>=rect.left && ci*10>=rect.top && cj*10<rect.right && ci*10<rect.bottom) return 1;
        if(cj*10>=rect.left && (ci+2)*10>=rect.top && cj*10<rect.right && (ci+2)*10<rect.bottom) return 1;
        return 0;
    }

    void Clicked(int x1, int y1, int x2, int y2){
        for(int i=0;i<laser_cnt;i++){
            if(laser[i].Create(x1, y1, x2, y2)) break;
        }
        return;
    }

    void ShootLaser(HWND hwnd){
        for(int i=0;i<laser_cnt;i++) laser[i].ChangePos(hwnd);
        for(int i=0;i<20;i++) parry_laser[i].ChangePos(hwnd);
        return;
    }

    void Show(HDC hdc){
        for(int i=0;i<laser_cnt;i++) laser[i].Show(hdc);
        for(int i=0;i<20;i++) parry_laser[i].Show(hdc);
        for(int i=0;i<2;i++) FillRect(hdc, &part[i].rect, part[i].color);
        FillRect(hdc, &bar_cover.rect, bar_cover.color);
        FillRect(hdc, &hp_bar.rect, hp_bar.color);
        if(parry_cd>=parry_MAXCD) DrawText(hdc, L"Parry", 5, &parry_status, DT_LEFT|DT_TOP);
        else DrawText(hdc, L"Can't Parry", 11, &parry_status, DT_LEFT|DT_TOP);
        return;
    }

    void ChangePos(int i, int j, RECT rect, HWND hwnd){
        if(i<0 || j<0 || i+2>=sizeY/10 || j>=sizeX/10 || Overlap(rect, i, j, hwnd)) return;
        InvalidateRect(hwnd, &part[0].rect, 1);
        InvalidateRect(hwnd, &part[1].rect, 1);
        assign_rect(&part[0].rect, j, i, j, i);
        assign_rect(&part[1].rect, j, i+1, j, i+2);
        assign_rect(&hitbox, j, i, j, i+2);
        InvalidateRect(hwnd, &part[0].rect, 1);
        InvalidateRect(hwnd, &part[1].rect, 1);
        left=j, top=i;
        return;
    }

    int Hit(RECT rect, HWND hwnd){
        int dmg=0;
        for(int i=0;i<laser_cnt;i++){
            if(laser[i].Overlap(rect, hwnd)){
                dmg+=atk;
                break;
            }
        }
        for(int i=0;i<20;i++){
            if(parry_laser[i].Overlap(rect, hwnd)){
                dmg+=atk*2;
                break;
            }
        }
        return dmg;
    }

    void Health(int z, int target_x, int target_y, HWND hwnd){
        if(z>0 && parrying){
            parry_cd = 10;
            parry_dur = 0;
            parrying = 0;
            for(int i=laser_cnt;i<20;i++){
                if(parry_laser[i].Create(left, top, target_x, target_y)) break;
            }
            InvalidateRect(hwnd, NULL, 1);
            return;
        }
        hp-=z;
        InvalidateRect(hwnd, &hp_bar.rect, 1);
        hp_bar.rect = {1, 1, round(hp/100.0*100.0)+1, 10};
        InvalidateRect(hwnd, &hp_bar.rect, 1);
        if(hp<=0){
            playing = 0;
            result = 0;
            InvalidateRect(hwnd, NULL, 1);
        }
        return;
    }

    void Parry(bool activated, HWND hwnd){
        parry_cd++;
        if(parrying) parry_dur++;
        if(parry_dur>=parry_MAXDUR){
            parrying = 0;
            parry_dur = parry_MAXDUR;
        }
        if(parry_cd>=parry_MAXCD) parry_cd = parry_MAXCD;
        InvalidateRect(hwnd, &parry_status, 1);
        if(parry_cd<parry_MAXCD) return;
        if(!activated) return;
        parry_cd = 0;
        parry_dur = 0;
        parrying = 1;
        InvalidateRect(hwnd, &parry_status, 1);
        return;
    }
};
void movePlr(PLAYER *plr, HWND hwnd);

struct ENTITY
{
    PIXEL part[8],hp_bar,bar_cover;
    int left,top,laser_cnt,hp,atk,attackTime,attackCd,BossAccuracy;
    LASER laser[100];
    RECT hitbox;
    ENTITY(){
        int offsetX=sizeX/2-55, offsetY=sizeY/2-50;
        laser_cnt = 40, hp = 500, atk = 5, attackTime = -1, attackCd = 1, BossAccuracy = 10;
        assign_pixel(&part[0], 0+offsetX, 0+offsetY, 100+offsetX, 100+offsetY, CreateSolidBrush(RGB(255, 62, 64)));
        assign_pixel(&part[1], 0+offsetX, 0+offsetY, 10+offsetX, 10+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&part[2], 10+offsetX, 10+offsetY, 10+offsetX, 100+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&part[3], 10+offsetX, 10+offsetY, 90+offsetX, 90+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&part[4], 30+offsetX, 30+offsetY, 70+offsetX, 70+offsetY, CreateSolidBrush(RGB(0, 0, 0)));
        assign_pixel(&part[5], 90+offsetX, 90+offsetY, 100+offsetX, 100+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&part[6], 90+offsetX, 0+offsetY, 100+offsetX, 10+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&part[7], 0+offsetX, 90+offsetY, 10+offsetX, 100+offsetY, CreateSolidBrush(RGB(255, 255, 255)));
        assign_pixel(&hp_bar, 1, 11, 101, 20, CreateSolidBrush(RGB(204, 0, 0)));
        assign_pixel(&bar_cover, 0, 10, 102, 21, CreateSolidBrush(RGB(255, 255, 255)));
        hitbox = {0+offsetX, 0+offsetY, 100+offsetX, 100+offsetY};
        left = part[4].rect.left/10, top = part[4].rect.top/10;
        for(int i=0;i<100;i++) laser[i].part.color = CreateSolidBrush(RGB(0, 255, 0));
    }

    ENTITY& operator=(const ENTITY &o){
        if (this!=&o){
            hp_bar = o.hp_bar;
            bar_cover = o.bar_cover;
            left = o.left, top = o.top;
            laser_cnt = o.laser_cnt;
            hp = o.hp;
            for(int i=0;i<100;i++) laser[i] = o.laser[i];
            hitbox = o.hitbox;
        }
        return *this;
    }

    void Show(HDC hdc){
        for(int i=0;i<laser_cnt;i++) laser[i].Show(hdc);
        for(int i=0;i<8;i++) FillRect(hdc, &part[i].rect, part[i].color);
        FillRect(hdc, &bar_cover.rect, bar_cover.color);
        FillRect(hdc, &hp_bar.rect, hp_bar.color);
        return;
    }

    void Attack(int plr_i, int plr_j, HWND hwnd){
        attackTime=(attackTime+1)%attackCd;
        for(int i=0;i<laser_cnt;i++) laser[i].ChangePos(hwnd);
        if(attackTime) return;
        for(int i=0;i<laser_cnt;i++){
            int ranX = plr_j+(rand()%BossAccuracy)*((rand()%2)?1:-1), ranY = plr_i+(rand()%BossAccuracy)*((rand()%2)?1:-1);
            if(laser[i].Create(part[4].rect.left/10, part[4].rect.top/10, ranX, ranY)) break;
        }
        return;
    }

    int Hit(RECT rect, HWND hwnd){
        for(int i=0;i<laser_cnt;i++){
            if(laser[i].Overlap(rect, hwnd)) return atk;
        }
        return 0;
    }

     void Health(int z, HWND hwnd){
        hp-=z;
        InvalidateRect(hwnd, &hp_bar.rect, 1);
        hp_bar.rect = {1, 11, round(hp/500.0*100.0)+1, 20};
        InvalidateRect(hwnd, &hp_bar.rect, 1);
        if(hp<=0){
            playing = 0;
            result = 1;
            InvalidateRect(hwnd, NULL, 1);
        }
        return;
    }
};

bool keyDown[1<<8];
RECT winrc;
POINT pt;
PLAYER plr;
ENTITY boss;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int coord_to_pixel(int op, int coord);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create a Window Class.
    /* The Window structure */
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    /* Use default icon and mouse-pointer */
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;                 /* No menu */
    wc.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wc.cbWndExtra = 0;                      /* structure or the window instance */

    /* Use Windows's default colour as the background of the window */
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    // Register the Window Class.
    if(!RegisterClassEx(&wc)){
        return 0;
    }

    // Create the Window.
    SystemParametersInfo(SPI_GETWORKAREA,0,&winrc,0);
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_POPUPWINDOW,                 // Window style

        (winrc.right-sizeX)/2, (winrc.bottom-sizeY)/2,      // Position
        sizeX, sizeY,                                       // Size

        HWND_DESKTOP,       // Parent window
        NULL,               // Menu
        hInstance,          // Instance handle
        NULL                // Additional application data
    );

    if(hwnd==NULL){
        return 0;
    }

    // Default Settings.
    for(int i=0;i<sizeY/10;i++){
        for(int j=0;j<sizeX/10;j++){
            board[i][j].rect = {j*10,i*10,j*10+10,i*10+10};
        }
    }
    srand(time(NULL));
    beginning = 1;

    // Show Window.
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void assign_rect(RECT *rect, int x1, int y1, int x2, int y2){
    SetRect(rect,
            board[y1][x1].rect.left,
            board[y1][x1].rect.top,
            board[y2][x2].rect.right,
            board[y2][x2].rect.bottom
           );
    return;
}

void assign_pixel(PIXEL *pixel, int left, int top, int right, int bottom, HBRUSH hbrush){
    pixel->rect = {left, top, right, bottom};
    pixel->color = hbrush;
    return;
}

void movePlr(PLAYER *plr, HWND hwnd){
    if(keyDown['W']) plr->ChangePos(plr->top-plr->walkSpeed, plr->left, boss.hitbox, hwnd);
    if(keyDown['A']) plr->ChangePos(plr->top, plr->left-plr->walkSpeed, boss.hitbox, hwnd);
    if(keyDown['S']) plr->ChangePos(plr->top+plr->walkSpeed, plr->left, boss.hitbox, hwnd);
    if(keyDown['D']) plr->ChangePos(plr->top, plr->left+plr->walkSpeed, boss.hitbox, hwnd);
    return;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch(uMsg)
    {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                SetBkMode(hdc, OPAQUEKEYBLOB);
                SetTextColor(hdc, RGB(255, 255, 255));
                // All painting occurs here, between BeginPaint and EndPaint.
                if(beginning){
                    RECT rect = {sizeX/2-200, sizeY/2-50, sizeX/2+200, sizeY/2+50};
                    FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
                    DrawText(hdc, L"GAME\nPRESS ANY KEY TO START\nWASD TO WALK\nF TO PARRY\nLEFT CLICK TO SHOOT", -1, &rect, DT_CENTER|DT_WORDBREAK);
                    EndPaint(hwnd, &ps);
                    return 0;
                }
                if(!playing){
                    KillTimer(hwnd, 1);
                    RECT rect = {sizeX/2-100, sizeY/2-50, sizeX/2+100, sizeY/2+50};
                    FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
                    if(result) DrawText(hdc, L"YOU WIN!\nENTER TO PLAY\nESC TO LEAVE", -1, &rect, DT_CENTER|DT_WORDBREAK);
                    else DrawText(hdc, L"YOU LOSE!\nENTER TO PLAY\nESC TO LEAVE", -1, &rect, DT_CENTER|DT_WORDBREAK);
                    EndPaint(hwnd, &ps);
                    return 0;
                }
                plr.Show(hdc);
                boss.Show(hdc);
                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_KEYDOWN:
            {
                if(beginning){
                    PLAYER plr_new;
                    ENTITY boss_new;
                    beginning = 0;
                    playing = 1;
                    plr = plr_new;
                    boss = boss_new;
                    SetTimer(hwnd, 1, 60, NULL);
                    InvalidateRect(hwnd, NULL, 1);
                    return 0;
                }
                if(!playing){
                    switch(wParam)
                    {
                        case 0x0D: // Enter
                            beginning = 1;
                            InvalidateRect(hwnd, NULL, 1);
                            break;

                        case 0x1B: // ESC
                            SendMessage(hwnd, WM_CLOSE, wParam, lParam);
                            break;
                    }
                    return 0;
                }
                if(wParam == 0x46){ // F
                    plr.Parry(1, hwnd);
                    return 0;
                }
                keyDown[wParam] = 1;
            }
            return 0;

        case WM_KEYUP:
            {
                keyDown[wParam] = 0;
            }
            return 0;

        case WM_LBUTTONDOWN:
            {
                if(!playing) return 0;
                int x1,y1,x2,y2;
                if(!GetCursorPos(&pt)) return 0;
                x1 = plr.left, y1 = plr.top;
                x2 = (pt.x-(winrc.right-sizeX)/2)/10, y2 = (pt.y-(winrc.bottom-sizeY)/2)/10;
                plr.Clicked(x1, y1, x2, y2);
                boss.Health(plr.Hit(boss.hitbox, hwnd), hwnd);
            }
            return 0;

        case WM_TIMER:
            {
                movePlr(&plr, hwnd);
                plr.Parry(0, hwnd);
                plr.ShootLaser(hwnd);
                boss.Health(plr.Hit(boss.hitbox, hwnd), hwnd);
                boss.Attack(plr.top, plr.left, hwnd);
                plr.Health(boss.Hit(plr.hitbox, hwnd), boss.left, boss.top, hwnd);
            }
            return 0;

        case WM_DESTROY:
            {
                KillTimer(hwnd, 1);
                PostQuitMessage(0);
            }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
