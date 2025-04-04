/**
 * 动态分区管理模拟程序
 * 实现最先适应、最佳适应和最坏适应三种内存分配算法
 */

#include <stdio.h>       // 标准输入输出库
#include <stdlib.h>      // 标准库函数，提供内存分配和程序控制功能
#include <string.h>      // 字符串处理函数库
#include <windows.h>     // Windows API函数库，用于控制台操作
#include <time.h>        // 时间函数库，用于随机数生成
#include <math.h>        // 用于abs()函数

// 内存分区状态常量定义
#define FREE 0    // 空闲状态标识
#define BUSY 1    // 已分配状态标识

// 内存分配算法类型常量定义
#define FIRST_FIT 1   // 最先适应算法标识
#define BEST_FIT  2   // 最佳适应算法标识
#define WORST_FIT 3   // 最坏适应算法标识

// 内存分区表项结构定义
typedef struct partition {
    int start_addr;        // 分区起始地址
    int size;              // 分区大小(KB)
    int status;            // 分区状态：FREE或BUSY
    char process_name[20]; // 占用该分区的进程名称
    struct partition *next;// 指向下一个分区的指针，形成链表结构
} Partition;

// 资源请求表项结构定义
typedef struct {
    char process_name[20]; // 请求分配内存的进程名称
    int size;              // 进程请求的内存大小(KB)
} Request;

// 全局变量定义
Partition *memory_list = NULL;  // 内存分区链表头指针
int total_memory_size = 1024;   // 总内存大小，默认为1024KB
int algorithm = FIRST_FIT;      // 当前使用的内存分配算法，默认为最先适应算法

// 函数声明
void initialize_memory();                  // 初始化内存
void display_memory();                     // 显示内存使用情况
int allocate_memory(Request req);          // 分配内存
int release_memory(char *process_name);    // 释放内存
Partition* first_fit(int size);            // 最先适应算法
Partition* best_fit(int size);             // 最佳适应算法
Partition* worst_fit(int size);            // 最坏适应算法
void merge_free_partitions();              // 合并相邻空闲分区
void print_menu();                         // 打印菜单
void clear_screen();                       // 清屏
void set_text_color(int color);            // 设置文本颜色
void reset_text_color();                   // 重置文本颜色
void set_console_charset();                // 设置控制台字符集

/**
 * 主函数：程序入口点，实现用户交互和功能调用
 */
int main() {
    int choice, ret;           // choice存储用户选择，ret存储函数返回结果
    Request req;               // 内存请求结构
    char process_name[20];     // 进程名称缓冲区
    
    // 设置控制台字符集，解决中文显示问题
    set_console_charset();
    
    // 初始化内存分区链表
    initialize_memory();
    
    // 主循环，实现用户交互
    while (1) {
        print_menu();          // 显示功能菜单
        printf("请选择操作: ");
        scanf("%d", &choice);  // 读取用户选择
        
        // 根据用户选择执行相应的操作
        switch (choice) {
            case 1: // 显示内存使用情况
                display_memory(); //调用 display_memory() 函数，显示当前的内存分配状态
                break;
                
            case 2: // 分配内存
                printf("请输入进程名: ");
                scanf("%s", req.process_name);     // 读取进程名
                printf("请输入所需内存大小(KB): ");
                scanf("%d", &req.size);            // 读取请求内存大小
                
                // 调用内存分配函数并处理结果
                ret = allocate_memory(req);//调用 allocate_memory 函数尝试分配内存。req 是请求的内存大小（通常是一个整数，表示所需内存的字节数）
                if (ret) { //检查 ret 的值以确定内存分配是否成功。如果 ret 不为 NULL（即非零），则表示内存分配成功
                    set_text_color(10); // 绿色，表示成功
                    printf("内存分配成功!\n");
                    reset_text_color(); //调用 reset_text_color 函数，重置文本颜色到默认值。这通常是在输出成功消息后，确保后续的输出不会继续使用绿色
                } else {
                    set_text_color(12); // 红色，表示失败
                    printf("内存分配失败，没有足够的空间!\n");
                    reset_text_color();
                }
                display_memory();  // 显示分配后的内存情况
                break;
                
            case 3: // 释放内存
                printf("请输入要释放内存的进程名: ");
                scanf("%s", process_name);   // 读取要释放内存的进程名
                
                // 调用内存释放函数并处理结果
                ret = release_memory(process_name);//调用 release_memory 函数，尝试释放与指定进程名称相关联的内存。函数返回值存储在 ret 变量中，通常是一个布尔值，指示释放操作是否成功
                if (ret) { //如果 ret 为真（非零），则表示内存释放成功
                    set_text_color(10); // 绿色，表示成功
                    printf("内存释放成功!\n");
                    reset_text_color();
                } else {
                    set_text_color(12); // 红色，表示失败 将文本颜色设置为红色（通常表示错误或失败）
                    printf("未找到该进程所占用的内存!\n");
                    reset_text_color();
                }
                display_memory();  // 显示释放后的内存情况
                break;
                
            case 4: // 切换内存分配算法
                printf("请选择分配算法 (1-最先适应, 2-最佳适应, 3-最坏适应): ");
                scanf("%d", &algorithm);
                // 验证算法选择是否有效 使用 scanf 函数从标准输入读取用户输入的分配算法编号，并将其存储在 algorithm 变量中。%d 格式说明符表示读取一个整数
                if (algorithm < 1 || algorithm > 3) {
                    algorithm = FIRST_FIT;  // 无效选择，默认为最先适应算法 检查用户输入的算法选择是否有效
                }
                
                // 显示当前使用的算法
                if (algorithm == FIRST_FIT) {
                    printf("当前使用算法: 最先适应\n");
                } else if (algorithm == BEST_FIT) {
                    printf("当前使用算法: 最佳适应\n");
                } else {
                    printf("当前使用算法: 最坏适应\n");
                }
                break;
                
            case 5: // 重置内存
                printf("正在重置内存...\n");
                // free(memory_list);        // 释放当前内存链表     调用 free 函数释放 memory_list 指向的内存链表。这是为了释放之前分配的内存，避免内存泄漏。
                initialize_memory();      // 重新初始化内存       这通常意味着将内存状态重置为初始状态，准备好进行新的内存分配。
                printf("内存已重置.\n");
                display_memory();         // 显示重置后的内存情况
                break;
                
            case 6: // 退出程序
                free(memory_list);        // 释放内存链表，避免内存泄漏
                printf("程序已退出.\n");
                exit(0);                  // 正常退出程序
                
            default:  // 处理无效输入
                printf("无效选择，请重新输入.\n");
        }
        
        // 等待用户按键继续
        printf("\n按任意键继续...");
        getchar(); // 吸收之前输入的回车
        getchar(); // 等待用户按键
        clear_screen();  // 清屏，准备下一轮操作
    }
    
    return 0;  // 程序正常结束
}

/**
 * 设置控制台字符集，解决中文乱码问题
 */
void set_console_charset() {
    // 设置控制台输出代码页为UTF-8
    SetConsoleOutputCP(65001);
    // 设置控制台输入代码页为UTF-8
    SetConsoleCP(65001);
    
    // 获取标准输出句柄                             //STD_OUTPUT_HANDLE 表示标准输出流（通常是控制台或终端的输出）
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //GetStdHandle 是 Windows API 函数，用于获取与指定标准流（例如标准输入、标准输出、标准错误）相关的句柄。
    if (hOut == INVALID_HANDLE_VALUE) {
        return;  // 获取句柄失败，直接返回  程序不再继续执行后续代码
    }
    
    // 设置控制台字体
    CONSOLE_FONT_INFOEX cfi; //声明一个 CONSOLE_FONT_INFOEX 结构体变量 cfi，该结构体用于存储控制台字体的相关信息。
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);  // 设置结构体大小      sizeof(CONSOLE_FONT_INFOEX) 获取结构体的字节大小
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);  // 获取当前字体信息
    
    // 尝试使用更好的中文显示字体
    wcscpy(cfi.FaceName, L"Consolas");  // 设置字体为Consolas 使用 wcscpy 函数将字符串 "Consolas"（这是一种常见的字体）复制到 cfi.FaceName 字段。
    cfi.dwFontSize.Y = 16;              // 设置字体大小    将字体的高度设置为 16 像素。dwFontSize 是一个 COORD 结构体，其中 Y 表示字体的高度，X 则表示字体的宽度。
    
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);  // 应用新字体设置  调用 SetCurrentConsoleFontEx 函数，应用之前修改过的字体设置（存储在 cfi 结构体中）。这会将控制台的字体更改为 Consolas，并将字体大小设置为 16 像素。
    
    // 另一种方法，如果上面的方法不起作用，可以尝试使用系统命令
    system("chcp 65001 > nul");  // 使用系统命令设置代码页，重定向输出避免干扰  
    //"chcp 65001" 是用来设置控制台的代码页为 UTF-8 编码（代码页 65001），有助于正确显示中文字符。
    // > nul 是将命令输出重定向到空设备，避免显示命令行输出干扰程序的正常执行
}

/**
 * 初始化内存，创建多个不连续空闲分区
 */
void initialize_memory() {
    Partition *last = NULL;
    Partition *new_partition = NULL;
    int segments = 4; // 创建的内存分区数量
    int available_memory = total_memory_size * 3 / 4; // 可用内存总量(总内存的75%)
    int segment_size = available_memory / segments; // 每个分区的基本大小
    int current_addr = 0; // 当前地址指针
    
    // 初始化随机数生成器
    // 不使用静态变量，每次都重新初始化
    srand((unsigned int)time(NULL));
    
    // 安全释放可能存在的旧内存链表
    Partition *p = memory_list;
    Partition *temp = NULL;
    
    while (p != NULL) {
        temp = p->next;  // 保存下一个节点
        free(p);         // 释放当前节点
        p = temp;        // 移动到下一个节点
    }
    memory_list = NULL;  // 重置链表头
    
    // 创建多个不连续的内存分区
    for (int i = 0; i < segments; i++) {
        // 分配新分区结构体
        new_partition = (Partition *)malloc(sizeof(Partition));
        if (!new_partition) {
            printf("内存分配失败！\n");
            exit(1);
        }
        
        // 设置分区大小(增加一些随机性，但限制范围防止异常)
        int size_variation = segment_size / 10; // 减小变化范围为基本大小的±10%
        int actual_size = segment_size;
        
        // 确保size_variation不为0，防止除以0错误
        if (size_variation > 0) {
            actual_size += (rand() % (2 * size_variation + 1) - size_variation);
        }
        
        // 确保大小为正数
        if (actual_size <= 0) {
            actual_size = segment_size;  // 出现异常时使用默认大小
        }
        
        // 初始化分区信息
        new_partition->start_addr = current_addr;
        new_partition->size = actual_size;
        new_partition->status = FREE;
        strcpy(new_partition->process_name, "空闲");
        new_partition->next = NULL;
        
        // 维护链表结构
        if (last) {
            last->next = new_partition;
        } else {
            memory_list = new_partition; // 设置链表头
        }
        last = new_partition;
        
        // 更新地址指针，添加间隙使得内存不连续
        int gap = 0;
        if (segment_size > 4) {  // 确保有足够空间生成间隙
            gap = 1 + (rand() % (segment_size / 4));  // 限制间隙大小，防止过大
        }
        current_addr += actual_size + gap;
    }
    
    // 添加一个较大的内存块在末尾
    new_partition = (Partition *)malloc(sizeof(Partition));
    if (!new_partition) {
        printf("内存分配失败！\n");
        exit(1);
    }
    
    new_partition->start_addr = current_addr;
    new_partition->size = total_memory_size / 4; // 最后一个分区占总内存的25%
    new_partition->status = FREE;
    strcpy(new_partition->process_name, "空闲");
    new_partition->next = NULL;
    
    if (last) {
        last->next = new_partition;
    } else {
        memory_list = new_partition;
    }
}

/**
 * 显示当前内存使用情况
 */
void display_memory() {
    Partition *p = memory_list;  // 从链表头开始遍历
    int i = 1;                   // 序号计数器
    
    // 打印表头
    printf("\n当前内存使用情况：\n");
    printf("--------------------------------------------------\n");
    printf("| 序号 | 起始地址 | 大小(KB) | 状态 | 进程名     |\n");
    printf("--------------------------------------------------\n");
    
    // 遍历链表并打印每个分区的信息
    while (p) {
        printf("| %-4d | %-8d | %-8d | %-4s | %-10s |\n", 
               i++,                                 // 内存分区的序号的变量        %-4d：输出一个整数，占用 4 个字符宽度，左对齐。
               p->start_addr,                       // 当前分区的起始地址          %-8d：输出一个整数，占用 8 个字符宽度，左对齐。
               p->size,                             // 大小                       %-4s：输出一个字符串，占用 4 个字符宽度，左对齐。
               p->status == FREE ? "空闲" : "已分配", // 状态                      %-10s：输出一个字符串，占用 10 个字符宽度，左对齐。
               p->process_name);                    // 进程名
        p = p->next;  // 移动到下一个分区
    }
    
    // 打印表尾
    printf("--------------------------------------------------\n");
}

/**
 * 分配内存函数
 * @param req 资源请求结构体，包含进程名和请求大小
 * @return 分配结果：1-成功，0-失败
 */
int allocate_memory(Request req) {
    Partition *target = NULL;        // 目标分区指针
    Partition *new_partition = NULL; // 新分区指针
    Partition *prev = NULL;          // 前一个分区指针，用于维护链表
    Partition *p = memory_list;      // 用于遍历链表
    
    // 根据当前算法选择合适的分区
    switch (algorithm) {
        case FIRST_FIT:  // 最先适应算法
            target = first_fit(req.size);
            break;
            
        case BEST_FIT:   // 最佳适应算法
            target = best_fit(req.size);
            break;
            
        case WORST_FIT:  // 最坏适应算法
            target = worst_fit(req.size);
            break;
            
        default:  // 默认使用最先适应算法
            target = first_fit(req.size);
    }
    
    // 如果找不到合适的分区，返回失败
    if (!target) {
        return 0;
    }
    
    // 找到目标分区在链表中的位置（找到prev）
    while (p && p != target) {
        prev = p;
        p = p->next;
    }
    
    // 如果找到的空闲分区恰好等于请求大小，直接分配
    if (target->size == req.size) {
        target->status = BUSY;                 // 设置状态为已分配
        strcpy(target->process_name, req.process_name); // 设置进程名
        return 1;  // 分配成功
    }
    
    // 如果找到的空闲分区大于请求大小，需要分割
    new_partition = (Partition *)malloc(sizeof(Partition));
    if (!new_partition) {
        printf("内存分配失败！\n");  // 分配新分区结构体失败
        return 0;
    }
    
    // 设置新分区的属性（已分配部分）
    new_partition->start_addr = target->start_addr;   // 新分区的起始地址
    new_partition->size = req.size;                   // 新分区的大小为请求大小
    new_partition->status = BUSY;                     // 状态为已分配
    strcpy(new_partition->process_name, req.process_name);  // 设置进程名
    
    // 修改原分区的属性（剩余空闲部分）
    target->start_addr += req.size;  // 更新原分区的起始地址
    target->size -= req.size;        // 更新原分区的大小
    
    // 正确插入新分区到链表中
    if (prev) {
        // 如果目标分区不是第一个节点
        prev->next = new_partition;  // 前一个节点指向新分区
        new_partition->next = target; // 新分区指向原分区
    } else {
        // 如果目标分区是第一个节点
        new_partition->next = target; // 新分区指向原分区
        memory_list = new_partition;  // 更新链表头
    }
    
    return 1;  // 分配成功
}

/**
 * 释放内存函数
 * @param process_name 要释放内存的进程名
 * @return 释放结果：1-成功，0-失败（未找到进程）
 */
int release_memory(char *process_name) { //函数接收一个 char *process_name 参数，这个参数是一个字符串，表示要释放内存的进程名称。
    Partition *p = memory_list;  // 从链表头开始遍历
    int found = 0;               // 标记是否找到匹配的进程
    
    // 查找并释放所有与process_name匹配的分区
    while (p) {
        if (p->status == BUSY && strcmp(p->process_name, process_name) == 0) { 
            //检查当前分区是否已经被分配（p->status == BUSY），并且该分区的进程名称是否与给定的 process_name 相匹配
            p->status = FREE;                // 设置状态为空闲
            strcpy(p->process_name, "空闲");  // 更新进程名为"空闲"
            found = 1;                       // 标记找到匹配的进程
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    // 如果找到并释放了内存，尝试合并相邻的空闲分区
    if (found) {
        merge_free_partitions();  // 合并相邻空闲分区
        return 1;  // 释放成功
    }
    
    return 0;  // 未找到匹配的进程
}

/**
 * 最先适应算法：查找第一个足够大的空闲分区
 * @param size 请求的内存大小
 * @return 找到的分区指针，如果没找到返回NULL
 */
Partition* first_fit(int size) {
    Partition *p = memory_list;  // 从链表头开始查找  p 用于遍历整个链表，查找符合条件的空闲内存分区
    
    // 遍历链表，查找第一个足够大的空闲分区
    while (p) {
        if (p->status == FREE && p->size >= size) {  //判断当前分区 p 是否为空闲 并且 该分区的大小是否大于或等于请求的内存大小
            return p;  // 找到合适分区，返回分区指针
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    return NULL;  // 没有找到合适的分区
}

/**
 * 最佳适应算法：查找最小的且足够大的空闲分区
 * @param size 请求的内存大小
 * @return 找到的分区指针，如果没找到返回NULL
 */
Partition* best_fit(int size) {
    Partition *p = memory_list;      // 从链表头开始查找
    Partition *best = NULL;          // 最佳匹配分区指针
    int min_size = total_memory_size + 1;  // 初始化为一个很大的值
    
    // 遍历链表，查找最佳匹配分区
    while (p) {
        if (p->status == FREE && p->size >= size) {   // 分区必须是空闲的且大小足够
            // 最佳适应是找到最小的足够大的分区，而不是差值最小
            if (p->size < min_size) {                 // 如果找到更小的合适分区
                min_size = p->size;                   // 更新最小分区大小
                best = p;                             // 更新最佳匹配分区
//                printf("%d",min_size);
            }
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    return best;  // 返回最佳分区或NULL
}

/**
 * 最坏适应算法：查找剩余空间最大的空闲分区
 * @param size 请求的内存大小
 * @return 找到的分区指针，如果没找到返回NULL
 */
Partition* worst_fit(int size) {
    Partition *p = memory_list;  // 从链表头开始查找
    Partition *worst = NULL;     // 最坏匹配分区指针
    int max_diff = -1;           // 初始化为一个很小的值   用于记录当前找到的分区与请求大小的差值
    
    // 遍历链表，查找最坏匹配分区
    while (p) {
        if (p->status == FREE && p->size >= size) {
            //检查当前分区是否为空闲 ，并且该分区的大小是否大于或等于请求的内存大小
            int diff = p->size - size;  // 计算当前分区与请求大小的差值
            if (diff > max_diff) {      // 如果差值更大，更新最坏匹配
                max_diff = diff;        // 更新最大差值        如果找到了更大的差值，更新 max_diff 为当前差值
                worst = p;              // 更新最坏匹配分区
            }
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    return worst;  // 返回最坏分区或NULL    返回找到的最坏匹配分区指针 worst
}

/**
 * 合并相邻的空闲分区
 */
void merge_free_partitions() {
    Partition *current, *next;
    int merged;

    do {
        merged = 0;
        current = memory_list;

        while (current && current->next) {
            next = current->next;

            // 检查当前分区和下一个分区是否都为空闲
            if (current->status == FREE && next->status == FREE) {
                // *** 新增检查：判断两个空闲分区在物理地址上是否连续 ***
                if (current->start_addr + current->size == next->start_addr) {
                    // 物理地址连续，可以合并
                    current->size += next->size;      // 增加当前分区的大小
                    current->next = next->next;       // 从链表中移除下一个分区
                    free(next);                       // 释放被合并分区的节点内存
                    merged = 1;                       // 标记发生了合并

                    // 合并后，当前分区可能能与再下一个分区合并，
                    // 因此使用 break 跳出内层循环，让外层 do-while 重新从头开始扫描
                    break;
                } else {
                    // 物理地址不连续，不能合并，继续检查下一对
                    current = current->next;
                }
            } else {
                // 至少有一个分区不是空闲，不能合并，继续检查下一对
                current = current->next;
            }
        }
        // 如果内层循环正常结束（没有break），表示本次扫描没有合并发生或已到链表尾部
    } while (merged); // 如果本次扫描发生了合并，则重新从头开始检查，确保所有可合并的都已合并
}

//两个 while 循环的组合，确保了：
//  在每次合并之后，链表会被重新检查，以确保所有相邻的空闲分区都被合并。
//  在每次遍历链表时，检查相邻的空闲分区并合并它们。

/**
 * 打印功能菜单
 */
void print_menu() {
    char *alg_name;  // 算法名称字符串，声明一个指向字符的指针 alg_name，用于存储当前选择的内存分配算法的名称
    
    // 根据当前算法设置算法名称
    if (algorithm == FIRST_FIT) {
        alg_name = "最先适应";
    } else if (algorithm == BEST_FIT) {
        alg_name = "最佳适应";
    } else {
        alg_name = "最坏适应";
    }
    
    // 打印菜单内容
    printf("\n======= 动态分区存储管理模拟 =======\n");
    printf("当前分配算法: %s\n", alg_name);
    printf("1. 显示内存使用情况\n");
    printf("2. 分配内存\n");
    printf("3. 释放内存\n");
    printf("4. 切换分配算法\n");
    printf("5. 重置内存\n");
    printf("6. 退出程序\n");
    printf("===================================\n");
}

/**
 * 清屏函数
 */
void clear_screen() {
    system("cls");  // Windows系统使用cls命令清屏
}

/**
 * 设置文本颜色
 * @param color 颜色代码
 */
void set_text_color(int color) {                        //接受一个 int color 参数，表示要设置的文本颜色       
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
    SetConsoleTextAttribute(hConsole, color);           // 设置文本颜色属性
    //调用 SetConsoleTextAttribute 函数来设置控制台文本的颜色。hConsole 是获取的控制台句柄，color 是传入的颜色代码
} 

/**
 * 重置文本颜色为默认值
 */
void reset_text_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄，为之后的 SetConsoleTextAttribute 函数提供控制台句柄
    SetConsoleTextAttribute(hConsole, 7);               // 设置为默认的白色(7)
} 