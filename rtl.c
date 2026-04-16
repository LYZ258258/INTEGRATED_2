// rtl.c
#include "rtl.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------- 工厂函数 --------------------- */
Signal* new_signal(const char* name, int line) {
    Signal* s = (Signal*)malloc(sizeof(Signal));
    strncpy(s->name, name, 63);
    s->name[63] = '\0';
    s->is_input = 0;
    s->is_output = 0;
    s->is_wire = 0;
    s->next = NULL;
    return s;
}

Gate* new_gate(const char* prefix, GateType type, int line) {
    static int gate_counter = 0;
    Gate* g = (Gate*)malloc(sizeof(Gate));
    
    const char* type_str = "UNKNOWN";
    switch (type) {
        case GATE_AND: type_str = "and"; break;
        case GATE_OR:  type_str = "or"; break;
        case GATE_XOR: type_str = "xor"; break;
        case GATE_NOT: type_str = "not"; break;
        case GATE_BUF: type_str = "buf"; break;
        case GATE_WIRE: type_str = "wire"; break;
    }
    sprintf(g->name, "%s_%s_%d", prefix, type_str, gate_counter++);
    
    g->type = type;
    g->output = NULL;
    g->inputs = NULL;
    g->input_count = 0;
    g->line = line;
    g->next = NULL;
    return g;
}

RTLNetlist* new_rtl_netlist(const char* module_name) {
    RTLNetlist* netlist = (RTLNetlist*)malloc(sizeof(RTLNetlist));
    strncpy(netlist->module_name, module_name, 63);
    netlist->module_name[63] = '\0';
    netlist->signals = NULL;
    netlist->gates = NULL;
    netlist->signal_count = 0;
    netlist->gate_count = 0;
    netlist->input_count = 0;
    netlist->output_count = 0;
    netlist->wire_count = 0;
    return netlist;
}

/* --------------------- 信号管理 --------------------- */
void add_signal_to_netlist(RTLNetlist* netlist, Signal* sig) {
    if (!netlist || !sig) return;
    
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (strcmp(s->name, sig->name) == 0) {
            s->is_input |= sig->is_input;
            s->is_output |= sig->is_output;
            s->is_wire |= sig->is_wire;
            free(sig);
            return;
        }
    }
    
    sig->next = netlist->signals;
    netlist->signals = sig;
    netlist->signal_count++;
}

Signal* find_signal(RTLNetlist* netlist, const char* name) {
    if (!netlist) return NULL;
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

Signal* find_or_create_signal(RTLNetlist* netlist, const char* name, int line) {
    Signal* sig = find_signal(netlist, name);
    if (!sig) {
        sig = new_signal(name, line);
        sig->is_wire = 1;
        add_signal_to_netlist(netlist, sig);
    }
    return sig;
}

/* --------------------- 门管理 --------------------- */
void add_gate_to_netlist(RTLNetlist* netlist, Gate* gate) {
    if (!netlist || !gate) return;
    gate->next = netlist->gates;
    netlist->gates = gate;
    netlist->gate_count++;
}

void gate_add_input(Gate* gate, Signal* sig) {
    if (!gate || !sig) return;
    gate->inputs = (Signal**)realloc(gate->inputs, 
                                     (gate->input_count + 1) * sizeof(Signal*));
    gate->inputs[gate->input_count] = sig;
    gate->input_count++;
}

/* --------------------- 表达式综合 --------------------- */
Signal* synthesize_expr(RTLNetlist* netlist, Expr* expr, int line) {
    if (!expr) return NULL;
    
    switch (expr->type) {
        case EXPR_ID: {
            return find_or_create_signal(netlist, expr->id_name, expr->line);
        }
        
        case EXPR_NUMBER: {
            char const_name[64];
            sprintf(const_name, "const_%d", expr->number);
            Signal* sig = find_signal(netlist, const_name);
            if (!sig) {
                sig = new_signal(const_name, expr->line);
                sig->is_wire = 1;
                add_signal_to_netlist(netlist, sig);
            }
            return sig;
        }
        
        case EXPR_BINARY_OP: {
            Signal* left_sig = synthesize_expr(netlist, expr->binop.left, line);
            Signal* right_sig = synthesize_expr(netlist, expr->binop.right, line);
            if (!left_sig || !right_sig) return NULL;
            
            GateType gate_type;
            const char* prefix;
            switch (expr->binop.op) {
                case T_AND: gate_type = GATE_AND; prefix = "and"; break;
                case T_OR:  gate_type = GATE_OR;  prefix = "or"; break;
                case T_XOR: gate_type = GATE_XOR; prefix = "xor"; break;
                default: return NULL;
            }
            
            Gate* gate = new_gate(prefix, gate_type, expr->line);
            char out_name[64];
            sprintf(out_name, "n%d_%s", netlist->signal_count, gate->name);
            Signal* out_sig = new_signal(out_name, expr->line);
            out_sig->is_wire = 1;
            add_signal_to_netlist(netlist, out_sig);
            
            gate->output = out_sig;
            gate_add_input(gate, left_sig);
            gate_add_input(gate, right_sig);
            add_gate_to_netlist(netlist, gate);
            
            return out_sig;
        }
        
        default:
            return NULL;
    }
}

/* --------------------- AST 到 RTL 转换 --------------------- */
RTLNetlist* ast_to_rtl(Module* mod) {
    if (!mod) return NULL;
    
    RTLNetlist* netlist = new_rtl_netlist(mod->name);
    
    printf("\n=== Synthesizing module '%s' ===\n", mod->name);
    
    // 1. 处理端口
    for (Port* p = mod->ports; p; p = p->next) {
        Signal* sig = find_signal(netlist, p->name);
        if (!sig) {
            sig = new_signal(p->name, p->line);
            add_signal_to_netlist(netlist, sig);
        }
        if (p->dir == PORT_INPUT) {
            sig->is_input = 1;
        } else if (p->dir == PORT_OUTPUT) {
            sig->is_output = 1;
        }
    }
    
    // 2. 处理 wire 声明
    for (VarDecl* v = mod->wires; v; v = v->next) {
        Signal* sig = find_or_create_signal(netlist, v->name, v->line);
        sig->is_wire = 1;
    }
    
    // 3. 处理 assign 语句
    for (AssignStmt* a = mod->assigns; a; a = a->next) {
        Signal* rhs_sig = synthesize_expr(netlist, a->rhs, a->line);
        Signal* lhs_sig = find_or_create_signal(netlist, a->lhs, a->line);
        
        if (rhs_sig && lhs_sig && rhs_sig != lhs_sig) {
            Gate* buf = new_gate("buf", GATE_BUF, a->line);
            buf->output = lhs_sig;
            gate_add_input(buf, rhs_sig);
            add_gate_to_netlist(netlist, buf);
        }
    }
    
    printf("=== Done: %d gates, %d signals ===\n", 
           netlist->gate_count, netlist->signal_count);

    // 重新统计
    netlist->input_count = 0;
    netlist->output_count = 0;
    netlist->wire_count = 0;
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (s->is_input) netlist->input_count++;
        if (s->is_output) netlist->output_count++;
        if (s->is_wire) netlist->wire_count++;
    }
    
    return netlist;
}

/* --------------------- 输出函数 --------------------- */
void print_rtl_netlist(RTLNetlist* netlist) {
    if (!netlist) return;
    
    printf("\n=== RTL Netlist: %s ===\n", netlist->module_name);
    printf("Signals: %d (in:%d out:%d wire:%d)\n", 
           netlist->signal_count, netlist->input_count, 
           netlist->output_count, netlist->wire_count);
    printf("Gates: %d\n", netlist->gate_count);
    
    for (Gate* g = netlist->gates; g; g = g->next) {
        printf("  %s: ", g->name);
        for (int i = 0; i < g->input_count; i++) {
            printf("%s ", g->inputs[i]->name);
        }
        printf("-> %s\n", g->output->name);
    }
}

void write_verilog_netlist(RTLNetlist* netlist, const char* filename) {
    if (!netlist || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "module %s(", netlist->module_name);
    int first = 1;
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (s->is_input || s->is_output) {
            if (!first) fprintf(f, ", ");
            fprintf(f, "%s", s->name);
            first = 0;
        }
    }
    fprintf(f, ");\n\n");
    
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (s->is_input) fprintf(f, "  input %s;\n", s->name);
    }
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (s->is_output) fprintf(f, "  output %s;\n", s->name);
    }
    for (Signal* s = netlist->signals; s; s = s->next) {
        if (s->is_wire) fprintf(f, "  wire %s;\n", s->name);
    }
    
    fprintf(f, "\n");
    for (Gate* g = netlist->gates; g; g = g->next) {
        if (g->type == GATE_AND) fprintf(f, "  and %s (", g->name);
        else if (g->type == GATE_OR) fprintf(f, "  or %s (", g->name);
        else if (g->type == GATE_XOR) fprintf(f, "  xor %s (", g->name);
        else if (g->type == GATE_BUF) fprintf(f, "  buf %s (", g->name);
        else continue;
        
        fprintf(f, "%s", g->output->name);
        for (int i = 0; i < g->input_count; i++) {
            fprintf(f, ", %s", g->inputs[i]->name);
        }
        fprintf(f, ");\n");
    }
    fprintf(f, "\nendmodule\n");
    fclose(f);
    printf("Wrote %s\n", filename);
}

/* --------------------- 内存释放 --------------------- */
void free_rtl_netlist(RTLNetlist* netlist) {
    if (!netlist) return;
    
    Signal* s = netlist->signals;
    while (s) { Signal* next = s->next; free(s); s = next; }
    
    Gate* g = netlist->gates;
    while (g) { 
        if (g->inputs) free(g->inputs);
        Gate* next = g->next; 
        free(g); 
        g = next; 
    }
    
    free(netlist);
}