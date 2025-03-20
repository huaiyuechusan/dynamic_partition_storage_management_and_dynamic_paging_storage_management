/**
 * 请求式分页管理模拟程序
 * 实现局部置换的FIFO页面置换算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>

// 系统参数定义
#define MEMORY_SIZE 64 * 1024      // 内存大小：64KB
#define BLOCK_SIZE 1024            // 内存块大小：1KB
#define MAX_PAGES 64               // 最大页数：64页
#define BLOCKS_PER_JOB 4           // 每个作业分配的内存块数
#define MAX_JOBS 1                 // 最大作业数

// 页表项结构
typedef struct {
    int page_number;      // 页号
    int present;          // 存在标志
    int frame_number;     // 内存块号
    int modified;         // 修改标志
    int disk_location;    // 磁盘位置
    int load_time;        // 页面装入时间（用于FIFO算法）
} PageTableEntry;

// 指令结构
typedef struct {
    char operation;       // 操作类型
    int page_number;      // 页号
    int offset;          // 页内地址
} Instruction;

// 全局变量
PageTableEntry page_table[MAX_PAGES];  // 页表
int memory_blocks[BLOCKS_PER_JOB];     // 作业分配的内存块
int current_time = 0;                  // 当前时间（用于FIFO算法）
int next_free_block = 0;               // 下一个空闲内存块

// 函数声明
void initialize_page_table();
void initialize_memory_blocks();
void display_page_table();
void display_instruction_info(int seq, Instruction inst, int physical_addr, int page_fault, int victim_page);
int get_physical_address(Instruction inst);
int handle_page_fault(int page_number);
int find_victim_page();
void save_page_to_disk(int page_number);
void load_page_from_disk(int page_number, int frame_number);
void set_text_color(int color);
void reset_text_color();
void set_console_charset();
void clear_screen();

// 主函数
int main() {
    // 设置控制台字符集
    set_console_charset();
    
    // 定义指令序列
    Instruction instructions[] = {
        {'+', 0, 72}, {'/', 1, 50}, {'x', 2, 15},
        {'s', 3, 26}, {'l', 0, 56}, {'-', 6, 40},
        {'+', 4, 56}, {'-', 5, 23}, {'s', 1, 37},
        {'+', 2, 78}, {'-', 4, 1}, {'s', 6, 86}
    };
    
    int physical_addr;
    int page_fault;
    int victim_page;
    
    // 记录每条指令的执行结果
    int result_addresses[12];
    int result_page_faults[12];
    int result_victims[12];
    
    // 初始化页表和内存块
    initialize_page_table();
    initialize_memory_blocks();
    current_time = 4; // 初始4个页面的装入时间为0-3
    
    printf("\n======= 请求式分页管理模拟 =======\n");
    printf("系统参数：\n");
    printf("内存大小：%d KB\n", MEMORY_SIZE / 1024);
    printf("内存块大小：%d 字节\n", BLOCK_SIZE);
    printf("每个作业分配的内存块数：%d\n", BLOCKS_PER_JOB);
    printf("===================================\n\n");
    
    // 显示初始页表
    printf("初始页表状态：\n");
    display_page_table();
    printf("\n按回车键继续...");
    while (getchar() != '\n') {  // 清空输入缓冲区
        continue;
    }
    
    // 执行指令序列
    for (int i = 0; i < 12; i++) {
        clear_screen();  // 每次迭代清屏
        
        printf("\n======= 请求式分页管理模拟 =======\n");
        printf("执行指令 %d: %c 页号=%d 页内地址=%d\n", 
               i + 1, instructions[i].operation, 
               instructions[i].page_number, instructions[i].offset);
        
        // 判断是否缺页
        page_fault = !page_table[instructions[i].page_number].present;
        
        // 获取物理地址(如果缺页会触发页面置换)
        if (page_fault) {
            victim_page = handle_page_fault(instructions[i].page_number);
            physical_addr = get_physical_address(instructions[i]);
            
            printf("执行结果：发生缺页中断");
            if (victim_page != -1) {
                printf("，淘汰第%d页\n", victim_page);
            } else {
                printf("\n");
            }
        } else {
            physical_addr = get_physical_address(instructions[i]);
            printf("执行结果：不缺页\n");
        }
        
        
        // 记录结果
        result_addresses[i] = physical_addr;
        result_page_faults[i] = page_fault;
        result_victims[i] = victim_page;
        
        // 显示指令执行信息
        display_instruction_info(i + 1, instructions[i], physical_addr, page_fault, victim_page);
        
        // 更新修改标志
        if (instructions[i].operation == 's') {
            page_table[instructions[i].page_number].modified = 1;
        }
        
        printf("\n当前页表状态：\n");
        display_page_table();
        
        printf("\n按回车键继续...");
        while (getchar() != '\n') {  // 清空输入缓冲区
            continue;
        }
    }
    
    printf("\n程序执行总结：\n");
    printf("-------------------------------------------------------------------\n");
    printf("序号\t物理地址\t缺页情况\t\t序号\t物理地址\t缺页情况\n");
    printf("-------------------------------------------------------------------\n");
    
    // 前6条指令
    for (int i = 0; i < 6; i++) {
        // 前6条指令
        printf("（%d）\t%d\t", i+1, result_addresses[i]);
        
        if (result_page_faults[i]) {
            if (result_victims[i] != -1) {
                printf("缺页，淘汰第%d页\t", result_victims[i]);
            } else {
                printf("缺页\t\t\t");
            }
        } else {
            printf("不缺页\t\t\t");
        }
        
        // 后6条指令
        printf("（%d）\t%d\t", i+7, result_addresses[i+6]);
        
        if (result_page_faults[i+6]) {
            if (result_victims[i+6] != -1) {
                printf("缺页，淘汰第%d页\n", result_victims[i+6]);
            } else {
                printf("缺页\n");
            }
        } else {
            printf("不缺页\n");
        }
    }
    
    printf("-------------------------------------------------------------------\n");
    
    printf("\n程序执行完毕，按回车键退出...");
    getchar();
    
    return 0;
}

// 初始化页表
void initialize_page_table() {
    // 初始化所有页表项
    for (int i = 0; i < MAX_PAGES; i++) {
        page_table[i].page_number = i;
        page_table[i].present = 0;
        page_table[i].frame_number = -1;
        page_table[i].modified = 0;
        page_table[i].disk_location = 0;
        page_table[i].load_time = 0;
    }
    
    // 设置初始页表状态，与题目要求一致
    // 页号  标志 内存块号 修改标志 在磁盘上的位置
    // 0     1    5        0        010
    // 1     1    8        0        012
    // 2     1    9        0        013
    // 3     1    1        0        021
    // 4     0              0        022
    // 5     0              0        023
    // 6     0              0        125
    
    // 前4页在内存中
    page_table[0].present = 1;
    page_table[0].frame_number = 5;
    page_table[0].disk_location = 10;
    page_table[0].load_time = 0;
    
    page_table[1].present = 1;
    page_table[1].frame_number = 8;
    page_table[1].disk_location = 12;
    page_table[1].load_time = 1;
    
    page_table[2].present = 1;
    page_table[2].frame_number = 9;
    page_table[2].disk_location = 13;
    page_table[2].load_time = 2;
    
    page_table[3].present = 1;
    page_table[3].frame_number = 1;
    page_table[3].disk_location = 21;
    page_table[3].load_time = 3;
    
    // 其余页在磁盘上
    page_table[4].disk_location = 22;
    page_table[5].disk_location = 23;
    page_table[6].disk_location = 125;
}

// 初始化内存块
void initialize_memory_blocks() {
    // 记录分配给作业的4个内存块
    memory_blocks[0] = 5; // 存放页0
    memory_blocks[1] = 8; // 存放页1
    memory_blocks[2] = 9; // 存放页2
    memory_blocks[3] = 1; // 存放页3
}

// 显示页表
void display_page_table() {
    printf("页号\t存在\t内存块号\t修改\t磁盘位置\t装入时间\n");
    printf("--------------------------------------------------------\n");
    for (int i = 0; i < MAX_PAGES; i++) {
        if (i < 7) {  
            printf("%d\t%d\t%d\t\t%d\t%d\t\t%d\n",
                   page_table[i].page_number,
                   page_table[i].present,
                   page_table[i].frame_number,
                   page_table[i].modified,
                   page_table[i].disk_location,
                   page_table[i].load_time);
        }
    }
    printf("--------------------------------------------------------\n");
}

// 显示指令执行信息
void display_instruction_info(int seq, Instruction inst, int physical_addr, int page_fault, int victim_page) {
    printf("\n指令执行详细信息：\n");
    printf("序号：%d\n", seq);
    printf("操作：%c\n", inst.operation);
    printf("页号：%d\n", inst.page_number);
    printf("页内地址：%d\n", inst.offset);
    printf("物理地址：%d\n", physical_addr);
    
    // 显示缺页情况
    if (page_fault) {
        set_text_color(12);  // 红色
        printf("缺页情况：发生缺页中断");
        if (victim_page != -1) {
            printf("，淘汰第%d页", victim_page);
        }
        reset_text_color();
    } else {
        set_text_color(10);  // 绿色
        printf("缺页情况：不缺页");
        reset_text_color();
    }
    printf("\n");
}

// 获取物理地址
int get_physical_address(Instruction inst) {
    int page_number = inst.page_number;
    int offset = inst.offset;
    
    // 计算物理地址，不检查页面是否在内存中（缺页已在外部处理）
    return (page_table[page_number].frame_number * BLOCK_SIZE) + offset;
}

// 处理缺页中断，返回被淘汰的页面号，如果没有淘汰页面则返回-1
int handle_page_fault(int page_number) {
    int victim_page = -1;
    int frame_number = -1;
    
    // 优化空闲块检测：直接跟踪已使用的内存块
    int used_blocks[BLOCKS_PER_JOB] = {0};
    for (int j = 0; j < MAX_PAGES; j++) {
        if (page_table[j].present) {
            for (int i = 0; i < BLOCKS_PER_JOB; i++) {
                if (page_table[j].frame_number == memory_blocks[i]) {
                    used_blocks[i] = 1;
                    break;
                }
            }
        }
    }
    
    // 查找第一个未使用的内存块
    for (int i = 0; i < BLOCKS_PER_JOB; i++) {
        if (!used_blocks[i]) {
            frame_number = memory_blocks[i];
            break;
        }
    }
    
    // 严格限制置换范围在作业分配的4个内存块中
    if (frame_number == -1) {
        victim_page = find_victim_page();
        frame_number = page_table[victim_page].frame_number;
        
        // 新增的淘汰页合法性检查
        int valid_victim = 0;
        for (int i = 0; i < BLOCKS_PER_JOB; i++) {
            if (frame_number == memory_blocks[i]) {
                valid_victim = 1;
                break;
            }
        }
        if (!valid_victim) {
            printf("错误：非法淘汰页 %d\n", victim_page);
            exit(1);
        }
    }
    
    // 如果有牺牲页且牺牲页被修改过，需要写回磁盘
    if (victim_page != -1 && page_table[victim_page].modified) {
        save_page_to_disk(victim_page);
    }
    
    // 更新页表，将被淘汰的页设为不在内存中
    if (victim_page != -1) {
        page_table[victim_page].present = 0;
        page_table[victim_page].frame_number = -1;
        page_table[victim_page].modified = 0;
    }
    
    // 将新页面装入内存
    load_page_from_disk(page_number, frame_number);
    
    // 更新页表
    page_table[page_number].present = 1;
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].load_time = current_time++;
    
    return victim_page;  // 返回被淘汰的页面号，如果没有淘汰页面则返回-1
}

// 增强find_victim_page的时间追踪
int find_victim_page() {
    int victim_page = -1;
    int earliest_time = INT_MAX;
    
    // 精确跟踪每个内存块的装入时间
    for (int i = 0; i < BLOCKS_PER_JOB; i++) {
        int current_block = memory_blocks[i];
        for (int j = 0; j < MAX_PAGES; j++) {
            if (page_table[j].present && page_table[j].frame_number == current_block) {
                if (page_table[j].load_time < earliest_time) {
                    earliest_time = page_table[j].load_time;
                    victim_page = j;
                }
                break; // 确保每个内存块只检查一次
            }
        }
    }
    
    return victim_page;
}

// 保存页面到磁盘
void save_page_to_disk(int page_number) {
    printf("将页面 %d 写回磁盘位置 %d\n", 
           page_number, page_table[page_number].disk_location);
    // 实际操作在这里不需要实现
}

// 从磁盘加载页面
void load_page_from_disk(int page_number, int frame_number) {
    printf("从磁盘位置 %d 加载页面 %d 到内存块 %d\n",
           page_table[page_number].disk_location, page_number, frame_number);
    // 实际操作在这里不需要实现
}

// 设置文本颜色
void set_text_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// 重置文本颜色
void reset_text_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);  // 默认为白色
}

// 设置控制台字符集
// 设置控制台字符集
void set_console_charset() {
    // 设置控制台输出代码页为UTF-8
    SetConsoleOutputCP(65001);
    // 设置控制台输入代码页为UTF-8
    SetConsoleCP(65001);
    
    // 获取标准输出句柄
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;
    }
    
    // 设置控制台字体
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    
    // 尝试使用更好的中文显示字体
    wcscpy(cfi.FaceName, L"Consolas");
    cfi.dwFontSize.Y = 16;
    
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    
    // 另一种方法，如果上面的方法不起作用，可以尝试使用系统命令
    system("chcp 65001 > nul");
}

// 添加清屏函数
void clear_screen() {
    system("cls");  // Windows系统使用cls命令清屏
} 