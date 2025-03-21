/**
 * 动态分区管理模拟程序
 * 实现最先适应、最佳适应和最坏适应三种内存分配算法
 */

#include <stdio.h>       // 标准输入输出库
#include <stdlib.h>      // 标准库函数，提供内存分配和程序控制功能
#include <string.h>      // 字符串处理函数库
#include <windows.h>     // Windows API函数库，用于控制台操作

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
                display_memory();
                break;
                
            case 2: // 分配内存
                printf("请输入进程名: ");
                scanf("%s", req.process_name);     // 读取进程名
                printf("请输入所需内存大小(KB): ");
                scanf("%d", &req.size);            // 读取请求内存大小
                
                // 调用内存分配函数并处理结果
                ret = allocate_memory(req);
                if (ret) {
                    set_text_color(10); // 绿色，表示成功
                    printf("内存分配成功!\n");
                    reset_text_color();
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
                ret = release_memory(process_name);
                if (ret) {
                    set_text_color(10); // 绿色，表示成功
                    printf("内存释放成功!\n");
                    reset_text_color();
                } else {
                    set_text_color(12); // 红色，表示失败
                    printf("未找到该进程所占用的内存!\n");
                    reset_text_color();
                }
                display_memory();  // 显示释放后的内存情况
                break;
                
            case 4: // 切换内存分配算法
                printf("请选择分配算法 (1-最先适应, 2-最佳适应, 3-最坏适应): ");
                scanf("%d", &algorithm);
                // 验证算法选择是否有效
                if (algorithm < 1 || algorithm > 3) {
                    algorithm = FIRST_FIT;  // 无效选择，默认为最先适应算法
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
                free(memory_list);        // 释放当前内存链表
                initialize_memory();      // 重新初始化内存
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
    
    // 获取标准输出句柄
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;  // 获取句柄失败，直接返回
    }
    
    // 设置控制台字体
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);  // 设置结构体大小
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);  // 获取当前字体信息
    
    // 尝试使用更好的中文显示字体
    wcscpy(cfi.FaceName, L"Consolas");  // 设置字体为Consolas
    cfi.dwFontSize.Y = 16;              // 设置字体大小
    
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);  // 应用新字体设置
    
    // 另一种方法，如果上面的方法不起作用，可以尝试使用系统命令
    system("chcp 65001 > nul");  // 使用系统命令设置代码页，重定向输出避免干扰
}

/**
 * 初始化内存，创建一个空闲分区
 */
void initialize_memory() {
    // 分配内存分区结构体
    memory_list = (Partition *)malloc(sizeof(Partition));
    if (!memory_list) {
        printf("内存分配失败！\n");  // 内存分配失败处理
        exit(1);                   // 异常退出程序
    }
    
    // 初始化分区信息
    memory_list->start_addr = 0;                 // 起始地址为0
    memory_list->size = total_memory_size;       // 大小为总内存大小
    memory_list->status = FREE;                  // 状态为空闲
    strcpy(memory_list->process_name, "空闲");    // 设置进程名为"空闲"
    memory_list->next = NULL;                    // 下一个节点为NULL
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
               i++,                                 // 序号
               p->start_addr,                       // 起始地址
               p->size,                             // 大小
               p->status == FREE ? "空闲" : "已分配", // 状态
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
    
    // 将新分区插入到链表中，新分区指向原分区
    new_partition->next = target;
    
    // 如果原分区是第一个节点，更新链表头
    if (target == memory_list) {
        memory_list = new_partition;  // 链表头更新为新分区
    } else {
        // 找到target的前一个节点
        Partition *p = memory_list;
        while (p && p->next != target) {
            p = p->next;
        }
        
        if (p) {
            p->next = new_partition;  // 前一个节点指向新分区
        }
    }
    
    return 1;  // 分配成功
}

/**
 * 释放内存函数
 * @param process_name 要释放内存的进程名
 * @return 释放结果：1-成功，0-失败（未找到进程）
 */
int release_memory(char *process_name) {
    Partition *p = memory_list;  // 从链表头开始遍历
    int found = 0;               // 标记是否找到匹配的进程
    
    // 查找并释放所有与process_name匹配的分区
    while (p) {
        if (p->status == BUSY && strcmp(p->process_name, process_name) == 0) {
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
    Partition *p = memory_list;  // 从链表头开始查找
    
    // 遍历链表，查找第一个足够大的空闲分区
    while (p) {
        if (p->status == FREE && p->size >= size) {
            return p;  // 找到合适分区，返回分区指针
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    return NULL;  // 没有找到合适的分区
}

/**
 * 最佳适应算法：查找大小最接近请求大小的空闲分区
 * @param size 请求的内存大小
 * @return 找到的分区指针，如果没找到返回NULL
 */
Partition* best_fit(int size) {
    Partition *p = memory_list;      // 从链表头开始查找
    Partition *best = NULL;          // 最佳匹配分区指针
    int min_diff = total_memory_size + 1;  // 初始化为一个很大的值
    
    // 遍历链表，查找最佳匹配分区
    while (p) {
        if (p->status == FREE && p->size >= size) {
            int diff = p->size - size;  // 计算当前分区与请求大小的差值
            if (diff < min_diff) {      // 如果差值更小，更新最佳匹配
                min_diff = diff;        // 更新最小差值
                best = p;               // 更新最佳匹配分区
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
    int max_diff = -1;           // 初始化为一个很小的值
    
    // 遍历链表，查找最坏匹配分区
    while (p) {
        if (p->status == FREE && p->size >= size) {
            int diff = p->size - size;  // 计算当前分区与请求大小的差值
            if (diff > max_diff) {      // 如果差值更大，更新最坏匹配
                max_diff = diff;        // 更新最大差值
                worst = p;              // 更新最坏匹配分区
            }
        }
        p = p->next;  // 继续检查下一个分区
    }
    
    return worst;  // 返回最坏分区或NULL
}

/**
 * 合并相邻的空闲分区
 */
void merge_free_partitions() {
    Partition *current, *next;  // 当前分区和下一个分区
    int merged;                 // 标记是否有合并操作
    
    // 循环直到没有合并操作为止
    do {
        merged = 0;             // 初始化标记为0
        current = memory_list;  // 从链表头开始
        
        // 遍历链表，查找可以合并的相邻空闲分区
        while (current && current->next) {
            next = current->next;  // 获取下一个分区
            
            // 如果当前分区和下一个分区都是空闲的，合并它们
            if (current->status == FREE && next->status == FREE) {
                current->size += next->size;      // 增加当前分区的大小
                current->next = next->next;       // 跳过下一个分区
                free(next);                       // 释放下一个分区的内存
                merged = 1;                       // 标记有合并操作
                break;                            // 中断当前循环，从头开始再次检查
            }
            
            current = current->next;  // 移动到下一个分区
        }
    } while (merged);  // 如果有合并操作，再次检查整个链表
}

/**
 * 打印功能菜单
 */
void print_menu() {
    char *alg_name;  // 算法名称字符串
    
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
void set_text_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
    SetConsoleTextAttribute(hConsole, color);           // 设置文本颜色属性
}

/**
 * 重置文本颜色为默认值
 */
void reset_text_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台句柄
    SetConsoleTextAttribute(hConsole, 7);               // 设置为默认的白色(7)
} 