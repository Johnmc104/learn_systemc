#include <iostream>
#include <sstream>
#include "systemc.h"

#define WIDTH       8
#define VEC_NUM     4
#define VEC_WIDTH   4

SC_MODULE(vector_mul) {
    sc_in<bool> clk, rst_n;
    sc_in<sc_int<WIDTH>> vec1[VEC_WIDTH],vec2[VEC_WIDTH];
    sc_out<sc_int<WIDTH * 2> > vec_o;

    void compute_vector_mul(void) {
        int temp = 0;
        if (rst_n.read() == false) {
            vec_o.write(0);
            return;
        }
        for (int i = 0; i < VEC_WIDTH; ++i) {
            temp = temp + vec1[i].read() * vec2[i].read();
        }
        vec_o.write(temp);
    };

    SC_CTOR(vector_mul) {
        SC_METHOD(compute_vector_mul);
        sensitive << clk.pos();
        sensitive << rst_n.neg();
    };
};

SC_MODULE(matrix_vector_mul) {
    sc_in<bool> clk,rst_n;
    sc_in<sc_int<WIDTH> > matrix[VEC_NUM][VEC_WIDTH];
    sc_in<sc_int<WIDTH> > vector_in[VEC_WIDTH];
    sc_out<sc_int<WIDTH * 2> > vector_out[VEC_NUM];

    vector_mul *pe[VEC_NUM];

    SC_CTOR(matrix_vector_mul) {
        std::ostringstream pe_name;
        for (int i = 0; i < VEC_NUM; ++i) {
            pe_name << "pe" << i;
            pe[i] = new vector_mul(pe_name.str().c_str());
            pe[i]->clk(clk);
            pe[i]->rst_n(rst_n);
            for (int j = 0; j < VEC_WIDTH; ++j) {
                pe[i]->vec1[j](matrix[i][j]);
                pe[i]->vec2[j](vector_in[j]);
            }
            pe[i]->vec_o(vector_out[i]);
            pe_name.str("");
        }
    };
};

SC_MODULE(driver) {
    sc_in <bool> clk;
    sc_out<bool> rst_n;
    sc_out<sc_int<WIDTH>> mat[VEC_NUM][VEC_WIDTH];
    sc_out<sc_int<WIDTH>> vec[VEC_WIDTH];

    void generate_input(void) {
        for (int i = 0; i < VEC_WIDTH; ++i) {
            for (int j = 0; j < VEC_NUM; ++j) {
                mat[j][i].write(rand() % ((int)pow(2,WIDTH) - 1));
            }
            vec[i].write(rand() % ((int)pow(2,WIDTH) - 1));
        }
        while(1) {
            wait();
            for (int i = 0; i < VEC_WIDTH; ++i) {
                for (int j = 0; j < VEC_NUM; ++j) {
                    mat[j][i].write(rand() % ((int)pow(2,WIDTH) - 1));
                }
                vec[i].write(rand() % ((int)pow(2,WIDTH) - 1));
            }
        }
    };

    void generate_reset(void) {
        rst_n.write(1);
        wait(1,SC_NS);
        rst_n.write(0);
        wait(1,SC_NS);
        rst_n.write(1);
    };

    SC_CTOR(driver) {
        SC_THREAD(generate_input);
        sensitive << clk.neg();
        SC_THREAD(generate_reset);
    };
};

int sc_main(int argc,char* argv[])
{
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst_n;
    sc_signal<sc_int<WIDTH> > mat[VEC_NUM][VEC_WIDTH], vec[VEC_WIDTH];
    sc_signal<sc_int<WIDTH * 2>>vec_o[VEC_NUM];

    sc_trace_file *fp;
    fp=sc_create_vcd_trace_file("wave");
    fp->set_time_unit(1, SC_NS);

    matrix_vector_mul dut("dut");
    dut.clk(clk);
    dut.rst_n(rst_n);
    for (int i = 0; i < VEC_NUM; ++i) {
        for (int j = 0; j < VEC_WIDTH; ++j) {
            dut.matrix[i][j](mat[i][j]);
        }
    }
    for (int i = 0; i < VEC_WIDTH; ++i) {
        dut.vector_in[i](vec[i]);
    }
    for (int i = 0; i < VEC_NUM; ++i) {
        dut.vector_out[i](vec_o[i]);
    }

    driver d("dri");
    d.clk(clk);
    d.rst_n(rst_n);
    for (int i = 0; i < VEC_WIDTH; ++i) {
        for (int j = 0; j < VEC_NUM; ++j) {
            d.mat[j][i](mat[j][i]);
        }
        d.vec[i](vec[i]);
    }

    sc_trace(fp,clk,"clk");
    sc_trace(fp,rst_n,"rst_n");
    for (int i = 0; i < VEC_NUM; ++i) {
        for (int j = 0; j < VEC_WIDTH; ++j) {
            std::ostringstream mat_name;
            mat_name << "matrix(" << i << "," << j << ")";
            sc_trace(fp,mat[i][j],mat_name.str());
            mat_name.str("");
        }
    }
    for (int i = 0; i < VEC_WIDTH; ++i) {
        std::ostringstream stream1;
        stream1 << "vec(" << i << ")";
        sc_trace(fp,vec[i],stream1.str());
        stream1.str("");
    }
    for (int i = 0; i < VEC_NUM; ++i) {
        std::ostringstream out_name;
        out_name << "dout(" << i << ")";
        sc_trace(fp,vec_o[i],out_name.str());
        out_name.str("");
    }

    sc_start(1000, SC_NS);

    sc_close_vcd_trace_file(fp);
    return 0;
};