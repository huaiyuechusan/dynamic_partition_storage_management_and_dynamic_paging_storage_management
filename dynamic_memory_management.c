/**
 * 动态分区管理模拟程序
 * 实现最先适应、最佳适应和最坏适应三种内存分配算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// 内存分区状态
#define FREE 0    // 空闲
#define BUSY 1    // 已分配

// 内存分配算法类型
#define FIRST_FIT 1   // 最先适应算法
#define BEST_FIT  2   // 最佳适应算法
#define WORST_FIT 3   // 最坏适应算法

// 内存分区表项结构
typedef struct partition {
    int start_addr;        // 分区起始地址
    int size;              // 分区大小
    int status;            // 分区状态：FREE或BUSY
    char process_name[20]; // 占用该分区的进程名称
    struct partition *next;// 指向下一个分区的指针
} Partition;

// 资源请求表项结构
typedef struct {
    char process_name[20]; // 进程名称
    int size;              // 请求内存大小
} Request;

// 全局变量
Partition *memory_list = NULL;  // 内存分区链表
int total_memory_size = 1024;   // 总内存大小，默认为1024KB
int algorithm = FIRST_FIT;      // 当前使用的内存分配算法

// 函数声明
void initialize_memory();
void display_memory();
int allocate_memory(Request req);
int release_memory(char *process_name);
Partition* first_fit(int size);
Partition* best_fit(int size);
Partition* worst_fit(int size);
void merge_free_partitions();
void print_menu();
void clear_screen();
void set_text_color(int color);
void reset_text_color();
void set_console_charset();

// 主函数
int main() {
    int choice, ret;
    Request req;
    char process_name[20];
    
    // 设置控制台字符集
    set_console_charset();
    
    // 初始化内存
    initialize_memory();
    
    while (1) {
        print_menu();
        printf("请选择操作: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: // 显示内存使用情况
                display_memory();
                break;
                
            case 2: // 分配内存
                printf("请输入进程名: ");
                scanf("%s", req.process_name);
                printf("请输入所需内存大小(KB): ");
                scanf("%d", &req.size);
                
                ret = allocate_memory(req);
                if (ret) {
                    set_text_color(10); // 绿色
                    printf("内存分配成功!\n");
                    reset_text_color();
                } else {
                    set_text_color(12); // 红色
                    printf("内存分配失败，没有足够的空间!\n");
                    reset_text_color();
                }
                display_memory();
                break;
                
            case 3: // 释放内存
                printf("请输入要释放内存的进程名: ");
                scanf("%s", process_name);
                
                ret = release_memory(process_name);
                if (ret) {
                    set_text_color(10); // 绿色
                    printf("内存释放成功!\n");
                    reset_text_color();
                } else {
                    set_text_color(12); // 红色
                    printf("未找到该进程所占用的内存!\n");
                    reset_text_color();
                }
                display_memory();
                break;
                
            case 4: // 切换内存分配算法
                printf("请选择分配算法 (1-最先适应, 2-最佳适应, 3-最坏适应): ");
                scanf("%d", &algorithm);
                if (algorithm < 1 || algorithm > 3) {
                    algorithm = FIRST_FIT;
                }
                
                if (algorithm == FIRST_FIT) {
                    printf("当前使用算法: 最先适应\n");
                } else if (algorithm == BEST_FIT) {
                    printf("当前使用算法: 最佳适应\n");
                } else {
                    printf("当前使用算法: 最坏适应\n");
                }
                break;
                
            case 5: // 重置内存
                free(memory_list);
                initialize_memory();
                printf("内存已重置.\n");
                display_memory();
                break;
                
            case 6: // 退出
                free(memory_list);
                printf("程序已退出.\n");
                exit(0);
                
            default:
                printf("无效选择，请重新输入.\n");
        }
        
        printf("\n按任意键继续...");
        getchar(); // 吸收之前输入的回车
        getchar(); // 等待用户按键
        clear_screen();
    }
    
    return 0;
}

// 设置控制台字符集，解决中文乱码问题
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

// 初始化内存，创建一个空闲分区
void initialize_memory() {
    memory_list = (Partition *)malloc(sizeof(Partition));
    if (!memory_list) {
        printf("内存分配失败！\n");
        exit(1);
    }
    
    memory_list->start_addr = 0;
    memory_list->size = total_memory_size;
    memory_list->status = FREE;
    strcpy(memory_list->process_name, "空闲");
    memory_list->next = NULL;
}

// 显示当前内存使用情况
void display_memory() {
    Partition *p = memory_list;
    int i = 1;
    
    printf("\n当前内存使用情况：\n");
    printf("--------------------------------------------------\n");
    printf("| 序号 | 起始地址 | 大小(KB) | 状态 | 进程名     |\n");
    printf("--------------------------------------------------\n");
    
    while (p) {
        printf("| %-4d | %-8d | %-8d | %-4s | %-10s |\n", 
               i++, 
               p->start_addr, 
               p->size, 
               p->status == FREE ? "空闲" : "已分配",
               p->process_name);
        p = p->next;
    }
    
    printf("--------------------------------------------------\n");
}

// 分配内存
int allocate_memory(Request req) {
    Partition *target = NULL;
    Partition *new_partition = NULL;
    
    // 根据当前算法选择合适的分区
    switch (algorithm) {
        case FIRST_FIT:
            target = first_fit(req.size);
            break;
            
        case BEST_FIT:
            target = best_fit(req.size);
            break;
            
        case WORST_FIT:
            target = worst_fit(req.size);
            break;
            
        default:
            target = first_fit(req.size);
    }
    
    // 如果找不到合适的分区，返回失败
    if (!target) {
        return 0;
    }
    
    // 如果找到的空闲分区恰好等于请求大小
    if (target->size == req.size) {
        target->status = BUSY;
        strcpy(target->process_name, req.process_name);
        return 1;
    }
    
    // 如果找到的空闲分区大于请求大小，需要分割
    new_partition = (Partition *)malloc(sizeof(Partition));
    if (!new_partition) {
        printf("内存分配失败！\n");
        return 0;
    }
    
    // 设置新分区的属性（已分配部分）
    new_partition->start_addr = target->start_addr;
    new_partition->size = req.size;
    new_partition->status = BUSY;
    strcpy(new_partition->process_name, req.process_name);
    
    // 修改原分区的属性（剩余空闲部分）
    target->start_addr += req.size;
    target->size -= req.size;
    
    // 将新分区插入到链表中
    new_partition->next = target;
    
    // 如果新分区是第一个节点
    if (target == memory_list) {
        memory_list = new_partition;
    } else {
        // 找到target的前一个节点
        Partition *p = memory_list;
        while (p && p->next != target) {
            p = p->next;
        }
        
        if (p) {
            p->next = new_partition;
        }
    }
    
    return 1;
}

// 释放内存
int release_memory(char *process_name) {
    Partition *p = memory_list;
    int found = 0;
    
    // 查找并释放所有与process_name匹配的分区
    while (p) {
        if (p->status == BUSY && strcmp(p->process_name, process_name) == 0) {
            p->status = FREE;
            strcpy(p->process_name, "空闲");
            found = 1;
        }
        p = p->next;
    }
    
    // 如果找到并释放了内存，尝试合并相邻的空闲分区
    if (found) {
        merge_free_partitions();
        return 1;
    }
    
    return 0;
}

// 最先适应算法
Partition* first_fit(int size) {
    Partition *p = memory_list;
    
    while (p) {
        if (p->status == FREE && p->size >= size) {
            return p;
        }
        p = p->next;
    }
    
    return NULL;  // 没有找到合适的分区
}

// 最佳适应算法
Partition* best_fit(int size) {
    Partition *p = memory_list;
    Partition *best = NULL;
    int min_diff = total_memory_size + 1;  // 初始化为一个很大的值
    
    while (p) {
        if (p->status == FREE && p->size >= size) {
            int diff = p->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best = p;
            }
        }
        p = p->next;
    }
    
    return best;  // 返回最佳分区或NULL
}

// 最坏适应算法
Partition* worst_fit(int size) {
    Partition *p = memory_list;
    Partition *worst = NULL;
    int max_diff = -1;  // 初始化为一个很小的值
    
    while (p) {
        if (p->status == FREE && p->size >= size) {
            int diff = p->size - size;
            if (diff > max_diff) {
                max_diff = diff;
                worst = p;
            }
        }
        p = p->next;
    }
    
    return worst;  // 返回最坏分区或NULL
}

// 合并相邻的空闲分区
void merge_free_partitions() {
    Partition *current, *next;
    int merged;
    
    do {
        merged = 0;
        current = memory_list;
        
        while (current && current->next) {
            next = current->next;
            
            // 如果当前分区和下一个分区都是空闲的，合并它们
            if (current->status == FREE && next->status == FREE) {
                current->size += next->size;  // 增加当前分区的大小
                current->next = next->next;   // 跳过下一个分区
                free(next);                   // 释放下一个分区的内存
                merged = 1;                   // 标记有合并操作
                break;                        // 从头开始再次检查
            }
            
            current = current->next;
        }
    } while (merged);  // 如果有合并操作，再次检查整个链表
}

// 打印菜单
void print_menu() {
    char *alg_name;
    
    if (algorithm == FIRST_FIT) {
        alg_name = "最先适应";
    } else if (algorithm == BEST_FIT) {
        alg_name = "最佳适应";
    } else {
        alg_name = "最坏适应";
    }
    
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

// 清屏函数
void clear_screen() {
    system("cls");  // Windows系统使用cls命令清屏
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