// rtl.h
#ifndef RTL_H
#define RTL_H

#include "ast.h"

/* --------------------- 门类型 --------------------- */
typedef enum {
    GATE_AND,      // 与门
    GATE_OR,       // 或门
    GATE_XOR,      // 异或门
    GATE_NOT,      // 非门
    GATE_BUF,      // 缓冲器
    GATE_WIRE      // 简单连线
} GateType;

/* --------------------- 信号节点 --------------------- */
typedef struct Signal {
    char name[64];
    int is_input;
    int is_output;
    int is_wire;
    struct Signal* next;
} Signal;

/* --------------------- 门实例 --------------------- */
typedef struct Gate {
    char name[64];
    GateType type;
    struct Signal* output;
    struct Signal** inputs;
    int input_count;
    int line;
    struct Gate* next;
} Gate;

/* --------------------- RTL 网表顶层 --------------------- */
typedef struct RTLNetlist {
    char module_name[64];
    Signal* signals;
    Gate* gates;
    int signal_count;
    int gate_count;
    int input_count;
    int output_count;
    int wire_count;
} RTLNetlist;

/* --------------------- 工厂函数 --------------------- */
Signal* new_signal(const char* name, int line);
Gate* new_gate(const char* prefix, GateType type, int line);
RTLNetlist* new_rtl_netlist(const char* module_name);

/* --------------------- 信号管理 --------------------- */
void add_signal_to_netlist(RTLNetlist* netlist, Signal* sig);
Signal* find_signal(RTLNetlist* netlist, const char* name);
Signal* find_or_create_signal(RTLNetlist* netlist, const char* name, int line);

/* --------------------- 门管理 --------------------- */
void add_gate_to_netlist(RTLNetlist* netlist, Gate* gate);
void gate_add_input(Gate* gate, Signal* sig);

/* --------------------- AST 到 RTL 转换 --------------------- */
RTLNetlist* ast_to_rtl(Module* mod);
Signal* synthesize_expr(RTLNetlist* netlist, Expr* expr, int line);

/* --------------------- 输出函数 --------------------- */
void print_rtl_netlist(RTLNetlist* netlist);
void write_verilog_netlist(RTLNetlist* netlist, const char* filename);
void write_dot_graph(RTLNetlist* netlist, const char* filename);

/* --------------------- 内存释放 --------------------- */
void free_rtl_netlist(RTLNetlist* netlist);

#endif