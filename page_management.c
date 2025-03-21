/**
 * 请求式分页管理模拟程序
 * 实现局部置换的FIFO页面置换算法
 */

/*
 * 解决方案分析：页面置换算法实现
 * 
 * 本代码实现了一个FIFO（先进先出）页面置换算法：
 * 1. 使用load_time跟踪每个页面进入内存的时间
 * 2. 当发生缺页且没有空闲内存块时，选择最早装入的页面进行置换
 * 3. 如果被置换的页面被修改过，需要将其写回磁盘
 * 4. 为确保局部置换，置换范围严格限制在作业分配的内存块内
 * 
 * 算法关键点：
 * - 维护页面的装入时间序列
 * - 在需要置换时选择时间最早的页面
 * - 只在作业自己的内存范围内进行置换
 */

#include <stdio.h>      // 提供标准输入输出函数
#include <stdlib.h>     // 提供内存分配和系统函数
#include <string.h>     // 提供字符串处理函数
#include <windows.h>    // 提供Windows系统API接口
#include <locale.h>     // 提供本地化设置函数

// 系统参数定义
#define MEMORY_SIZE 64 * 1024      // 内存大小：64KB
#define BLOCK_SIZE 1024            // 内存块大小：1KB
#define MAX_PAGES 64               // 最大页数：64页
#define BLOCKS_PER_JOB 4           // 每个作业分配的内存块数
#define MAX_JOBS 1                 // 最大作业数

// 页表项结构 - 每个页面在页表中的一个条目
typedef struct {
    int page_number;      // 页号 - 标识逻辑页面
    int present;          // 存在标志 - 1表示在内存中，0表示不在
    int frame_number;     // 内存块号 - 页面所在的物理内存块
    int modified;         // 修改标志 - 1表示被修改过，0表示未修改
    int disk_location;    // 磁盘位置 - 页面在磁盘上的位置
    int load_time;        // 页面装入时间（用于FIFO算法）- 记录页面被装入的时间点
} PageTableEntry;

// 指令结构 - 表示一条内存访问指令
typedef struct {
    char operation;       // 操作类型 - 如 '+', '-', 's', 'l' 等
    int page_number;      // 页号 - 要访问的页面
    int offset;           // 页内地址 - 页内偏移量
} Instruction;

// 全局变量
PageTableEntry page_table[MAX_PAGES];  // 页表 - 记录所有页面的状态信息
int memory_blocks[BLOCKS_PER_JOB];     // 作业分配的内存块 - 记录分配给作业的物理内存块
int current_time = 0;                  // 当前时间（用于FIFO算法）- 时间计数器
int next_free_block = 0;               // 下一个空闲内存块 - 跟踪可用的内存块

// 函数声明
void initialize_page_table();          // 初始化页表
void initialize_memory_blocks();       // 初始化内存块
void display_page_table();             // 显示页表状态
void display_instruction_info(int seq, Instruction inst, int physical_addr, int page_fault, int victim_page);  // 显示指令执行信息
int get_physical_address(Instruction inst);  // 获取物理地址
int handle_page_fault(int page_number);      // 处理缺页中断
int find_victim_page();                      // 查找要被置换的页面
void save_page_to_disk(int page_number);     // 保存页面到磁盘
void load_page_from_disk(int page_number, int frame_number);  // 从磁盘加载页面
void set_text_color(int color);              // 设置文本颜色
void reset_text_color();                     // 重置文本颜色
void set_console_charset();                  // 设置控制台字符集
void clear_screen();                         // 清屏

// 主函数 - 程序的入口点
int main() {
    // 设置控制台字符集
    set_console_charset();
    
    // 定义指令序列 - 模拟程序要执行的内存访问指令
    Instruction instructions[] = {
        {'+', 0, 72}, {'/', 1, 50}, {'x', 2, 15},
        {'s', 3, 26}, {'l', 0, 56}, {'-', 6, 40},
        {'+', 4, 56}, {'-', 5, 23}, {'s', 1, 37},
        {'+', 2, 78}, {'-', 4, 1}, {'s', 6, 86}
    };
    
    int physical_addr;  // 物理地址
    int page_fault;     // 缺页标志
    int victim_page;    // 被淘汰的页面
    
    // 记录每条指令的执行结果
    int result_addresses[12];    // 存储每条指令的物理地址
    int result_page_faults[12];  // 存储每条指令是否发生缺页
    int result_victims[12];      // 存储每条指令淘汰的页面
    
    // 初始化页表和内存块
    initialize_page_table();
    initialize_memory_blocks();
    current_time = 4; // 初始4个页面的装入时间为0-3
    
    // 打印系统参数信息
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
    
    // 执行指令序列 - 模拟12条指令的执行过程
    for (int i = 0; i < 12; i++) {
        clear_screen();  // 每次迭代清屏
        
        // 显示当前执行的指令
        printf("\n======= 请求式分页管理模拟 =======\n");
        printf("执行指令 %d: %c 页号=%d 页内地址=%d\n", 
               i + 1, instructions[i].operation, 
               instructions[i].page_number, instructions[i].offset);
        
        // 判断是否缺页 - 检查页面是否在内存中
        page_fault = !page_table[instructions[i].page_number].present;
        
        // 获取物理地址(如果缺页会触发页面置换)
        if (page_fault) {
            // 处理缺页中断，返回被淘汰的页面
            victim_page = handle_page_fault(instructions[i].page_number);
            physical_addr = get_physical_address(instructions[i]);
            
            // 显示缺页信息
            printf("执行结果：发生缺页中断");
            if (victim_page != -1) {
                printf("，淘汰第%d页\n", victim_page);
            } else {
                printf("\n");
            }
        } else {
            // 页面已在内存中，直接获取物理地址
            physical_addr = get_physical_address(instructions[i]);
            printf("执行结果：不缺页\n");
        }
        
        // 记录指令执行结果
        result_addresses[i] = physical_addr;
        result_page_faults[i] = page_fault;
        result_victims[i] = victim_page;
        
        // 显示指令执行详细信息
        display_instruction_info(i + 1, instructions[i], physical_addr, page_fault, victim_page);
        
        // 更新修改标志 - 如果是存储操作(s)，标记页面已修改
        if (instructions[i].operation == 's') {
            page_table[instructions[i].page_number].modified = 1;
        }
        
        // 显示当前页表状态
        printf("\n当前页表状态：\n");
        display_page_table();
        
        // 等待用户按回车继续
        printf("\n按回车键继续...");
        while (getchar() != '\n') {  // 清空输入缓冲区
            continue;
        }
    }
    
    // 打印程序执行总结
    printf("\n程序执行总结：\n");
    printf("-------------------------------------------------------------------\n");
    printf("序号\t物理地址\t缺页情况\t\t序号\t物理地址\t缺页情况\n");
    printf("-------------------------------------------------------------------\n");
    
    // 前6条指令和后6条指令并排显示
    for (int i = 0; i < 6; i++) {
        // 显示前6条指令的结果
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
        
        // 显示后6条指令的结果
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
    
    // 等待用户按回车退出
    printf("\n程序执行完毕，按回车键退出...");
    getchar();
    
    return 0;
}

// 初始化页表 - 设置页表的初始状态
void initialize_page_table() {
    // 初始化所有页表项
    for (int i = 0; i < MAX_PAGES; i++) {
        page_table[i].page_number = i;     // 设置页号
        page_table[i].present = 0;         // 初始默认不在内存中
        page_table[i].frame_number = -1;   // 没有分配内存块
        page_table[i].modified = 0;        // 初始未修改
        page_table[i].disk_location = 0;   // 初始磁盘位置为0
        page_table[i].load_time = 0;       // 初始装入时间为0
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
    page_table[0].present = 1;             // 页0在内存中
    page_table[0].frame_number = 5;        // 存放在内存块5
    page_table[0].disk_location = 10;      // 磁盘位置为10
    page_table[0].load_time = 0;           // 装入时间为0
    
    page_table[1].present = 1;             // 页1在内存中
    page_table[1].frame_number = 8;        // 存放在内存块8
    page_table[1].disk_location = 12;      // 磁盘位置为12
    page_table[1].load_time = 1;           // 装入时间为1
    
    page_table[2].present = 1;             // 页2在内存中
    page_table[2].frame_number = 9;        // 存放在内存块9
    page_table[2].disk_location = 13;      // 磁盘位置为13
    page_table[2].load_time = 2;           // 装入时间为2
    
    page_table[3].present = 1;             // 页3在内存中
    page_table[3].frame_number = 1;        // 存放在内存块1
    page_table[3].disk_location = 21;      // 磁盘位置为21
    page_table[3].load_time = 3;           // 装入时间为3
    
    // 其余页在磁盘上
    page_table[4].disk_location = 22;      // 页4在磁盘位置22
    page_table[5].disk_location = 23;      // 页5在磁盘位置23
    page_table[6].disk_location = 125;     // 页6在磁盘位置125
}

// 初始化内存块 - 设置作业分配的内存块
void initialize_memory_blocks() {
    // 记录分配给作业的4个内存块
    memory_blocks[0] = 5; // 存放页0的内存块
    memory_blocks[1] = 8; // 存放页1的内存块
    memory_blocks[2] = 9; // 存放页2的内存块
    memory_blocks[3] = 1; // 存放页3的内存块
}

// 显示页表 - 打印当前页表的状态
void display_page_table() {
    // 打印表头
    printf("页号\t存在\t内存块号\t修改\t磁盘位置\t装入时间\n");
    printf("--------------------------------------------------------\n");
    
    // 打印每个页表项的信息
    for (int i = 0; i < MAX_PAGES; i++) {
        if (i < 7) {  // 只显示前7页的信息
            printf("%d\t%d\t%d\t\t%d\t%d\t\t%d\n",
                   page_table[i].page_number,   // 页号
                   page_table[i].present,       // 存在标志
                   page_table[i].frame_number,  // 内存块号
                   page_table[i].modified,      // 修改标志
                   page_table[i].disk_location, // 磁盘位置
                   page_table[i].load_time);    // 装入时间
        }
    }
    printf("--------------------------------------------------------\n");
}

// 显示指令执行信息 - 打印指令执行的详细信息
void display_instruction_info(int seq, Instruction inst, int physical_addr, int page_fault, int victim_page) {
    printf("\n指令执行详细信息：\n");
    printf("序号：%d\n", seq);                 // 指令序号
    printf("操作：%c\n", inst.operation);      // 操作类型
    printf("页号：%d\n", inst.page_number);    // 页号
    printf("页内地址：%d\n", inst.offset);     // 页内偏移
    printf("物理地址：%d\n", physical_addr);   // 计算得到的物理地址
    
    // 显示缺页情况，使用不同颜色区分
    if (page_fault) {
        set_text_color(12);  // 红色 - 表示缺页
        printf("缺页情况：发生缺页中断");
        if (victim_page != -1) {
            printf("，淘汰第%d页", victim_page);  // 显示被淘汰的页面
        }
        reset_text_color();
    } else {
        set_text_color(10);  // 绿色 - 表示不缺页
        printf("缺页情况：不缺页");
        reset_text_color();
    }
    printf("\n");
}

// 获取物理地址 - 根据逻辑地址计算物理地址
int get_physical_address(Instruction inst) {
    int page_number = inst.page_number;  // 页号
    int offset = inst.offset;            // 页内偏移
    
    // 计算物理地址 = 内存块号 * 内存块大小 + 页内偏移
    return (page_table[page_number].frame_number * BLOCK_SIZE) + offset;
}

// 处理缺页中断 - 当页面不在内存时调用，返回被淘汰的页面号
int handle_page_fault(int page_number) {
    int victim_page = -1;   // 被淘汰的页面，初始为-1表示无淘汰页面
    int frame_number = -1;  // 分配的内存块号
    
    // 优化空闲块检测：直接跟踪已使用的内存块
    int used_blocks[BLOCKS_PER_JOB] = {0};  // 记录内存块使用情况，0表示未使用
    
    // 遍历页表，标记已使用的内存块
    for (int j = 0; j < MAX_PAGES; j++) {
        if (page_table[j].present) {  // 如果页面在内存中
            for (int i = 0; i < BLOCKS_PER_JOB; i++) {
                if (page_table[j].frame_number == memory_blocks[i]) {
                    used_blocks[i] = 1;  // 标记该内存块已使用
                    break;
                }
            }
        }
    }
    
    // 查找第一个未使用的内存块
    for (int i = 0; i < BLOCKS_PER_JOB; i++) {
        if (!used_blocks[i]) {  // 如果内存块未使用
            frame_number = memory_blocks[i];  // 使用该内存块
            break;
        }
    }
    
    // 如果没有空闲内存块，需要进行页面置换
    if (frame_number == -1) {
        victim_page = find_victim_page();  // 查找要被淘汰的页面
        frame_number = page_table[victim_page].frame_number;  // 使用淘汰页面的内存块
        
        // 新增的淘汰页合法性检查 - 确保淘汰的页面所在内存块属于作业
        int valid_victim = 0;
        for (int i = 0; i < BLOCKS_PER_JOB; i++) {
            if (frame_number == memory_blocks[i]) {
                valid_victim = 1;  // 淘汰页面有效
                break;
            }
        }
        if (!valid_victim) {  // 如果淘汰页面无效，报错退出
            printf("错误：非法淘汰页 %d\n", victim_page);
            exit(1);
        }
    }
    
    // 如果有牺牲页且牺牲页被修改过，需要写回磁盘
    if (victim_page != -1 && page_table[victim_page].modified) {
        save_page_to_disk(victim_page);  // 将修改过的页面写回磁盘
    }
    
    // 更新页表，将被淘汰的页设为不在内存中
    if (victim_page != -1) {
        page_table[victim_page].present = 0;        // 标记不在内存中
        page_table[victim_page].frame_number = -1;  // 清除内存块号
        page_table[victim_page].modified = 0;       // 清除修改标志
    }
    
    // 将新页面装入内存
    load_page_from_disk(page_number, frame_number);
    
    // 更新页表
    page_table[page_number].present = 1;               // 标记在内存中
    page_table[page_number].frame_number = frame_number; // 设置内存块号
    page_table[page_number].load_time = current_time++;  // 记录装入时间
    
    return victim_page;  // 返回被淘汰的页面号，如果没有淘汰页面则返回-1
}

// FIFO页面置换算法 - 查找最早装入的页面进行置换
int find_victim_page() {
    int victim_page = -1;        // 被淘汰的页面
    int earliest_time = INT_MAX; // 最早的装入时间
    
    // 精确跟踪每个内存块的装入时间
    for (int i = 0; i < BLOCKS_PER_JOB; i++) {
        int current_block = memory_blocks[i];  // 当前检查的内存块
        for (int j = 0; j < MAX_PAGES; j++) {
            // 找到在内存中且使用该内存块的页面
            if (page_table[j].present && page_table[j].frame_number == current_block) {
                // 如果该页面的装入时间早于当前最早时间
                if (page_table[j].load_time < earliest_time) {
                    earliest_time = page_table[j].load_time;  // 更新最早时间
                    victim_page = j;                         // 更新淘汰页面
                }
                break; // 确保每个内存块只检查一次
            }
        }
    }
    
    return victim_page;  // 返回要淘汰的页面
}

// 保存页面到磁盘 - 模拟将页面内容写回磁盘
void save_page_to_disk(int page_number) {
    printf("将页面 %d 写回磁盘位置 %d\n", 
           page_number, page_table[page_number].disk_location);
    // 实际操作在这里不需要实现，只是模拟
}

// 从磁盘加载页面 - 模拟从磁盘读取页面到内存
void load_page_from_disk(int page_number, int frame_number) {
    printf("从磁盘位置 %d 加载页面 %d 到内存块 %d\n",
           page_table[page_number].disk_location, page_number, frame_number);
    // 实际操作在这里不需要实现，只是模拟
}

// 设置文本颜色 - 更改控制台输出的颜色
void set_text_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
    SetConsoleTextAttribute(hConsole, color);           // 设置文本属性
}

// 重置文本颜色 - 将文本颜色恢复为默认值
void reset_text_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
    SetConsoleTextAttribute(hConsole, 7);               // 设置为默认白色
}

// 设置控制台字符集 - 配置控制台以正确显示中文等字符
void set_console_charset() {
    // 设置控制台输出代码页为UTF-8
    SetConsoleOutputCP(65001);
    // 设置控制台输入代码页为UTF-8
    SetConsoleCP(65001);
    
    // 获取标准输出句柄
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;  // 如果获取句柄失败，直接返回
    }
    
    // 设置控制台字体
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);          // 结构体大小
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);        // 获取当前字体信息
    
    // 尝试使用更好的中文显示字体
    wcscpy(cfi.FaceName, L"Consolas");                 // 设置字体为Consolas
    cfi.dwFontSize.Y = 16;                             // 设置字体大小
    
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);        // 应用字体设置
    
    // 另一种方法，如果上面的方法不起作用，可以尝试使用系统命令
    system("chcp 65001 > nul");                        // 设置代码页为UTF-8
}

// 清屏函数 - 清除控制台显示的内容
void clear_screen() {
    system("cls");  // Windows系统使用cls命令清屏
} 