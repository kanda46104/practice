#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "MT.h"

#define n0 5 //初期ネットワークのノード数

//接続時のデータ更新
int connect(int node1,int node2,int edges_count,int *kn,int **edges){
        //次数更新
        kn[node1]++;
        kn[node2]++;
        //エッジ更新
        edges[edges_count][0] = node1;
        edges[edges_count][1] = node2;
        edges_count++;

        return edges_count;
}

//BA選択用関数
int BA_select(int *kn,int node_count){
        int i;
        int k_sum; //次数合計
        int selection_node; //選択ノード
        double p; //選択確率

        //次数合計計算
        k_sum = 0;
        for(i = 1; i < node_count; i++) { k_sum += kn[i]; }

        //BA選択
        selection_node = -1;
        do {
                i=genrand_int32()%(node_count-1)+1;
                p = (double)kn[i]/(double)k_sum;
                if(genrand_real1() <= p) {         //確率でノード選択
                        selection_node = i;
                        break;
                }

        } while(selection_node == -1); //選択されるまで繰り返す

        return selection_node;
}


int main(void){
        init_genrand((unsigned)time(NULL));
        FILE *edgesf;
        int i,j;
        int flag;
        int n; //全ノード数
        int m; //ノード追加時の接続エッジ数
        int node_count=0;
        int edges_count=0;
        int edges_max;
        int reiterate_N;
        int reiterate_count;

        char s1[20];

        clock_t time_start_all,time_end_all;

        time_start_all=clock();

        //パラメータの入力
        printf("node_size >>\n");
        scanf("%d", &n);
        printf("m >>\n");
        scanf("%d", &m);
        printf("network_size >>\n");
        scanf("%d",&reiterate_N);

        edges_max = (n-n0)*m + n0*(n0-1)/2; //最大エッジ数

        //次数用のメモリ確保
        int *kn;
        kn = (int*)calloc(n+1, sizeof(int));
        if(kn == NULL) { printf("error 2\n"); exit(1); }
        //接続ノード保存用のメモリ確保
        int *partner;
        partner = (int*)calloc(m,sizeof(int));
        if(partner == NULL) { printf("error 2\n"); exit(1); }
        //エッジリスト用のメモリ確保
        int **edges;
        edges = (int**)calloc(edges_max,sizeof(int*));
        if(edges == NULL) { printf("error 2\n"); exit(1); }
        for(i = 0; i < edges_max; i++) {
                edges[i] = (int*)calloc(2,sizeof(int));
                if(edges[i] == NULL) { printf("error 2\n"); exit(1); }
        }

        for(reiterate_count=0; reiterate_count<reiterate_N; reiterate_count++) {

                //初期化
                for(i=0; i<n+1; i++) { kn[i]=0; }
                for(i=0; i<m; i++) { partner[i]=0; }
                for(i=0; i<edges_max; i++) { for(j=0; j<2; j++) { edges[i][j]=0; } }

                //初期ネットワーク生成　完全グラフ//
                node_count = n0;
                edges_count = 0;
                for(i = 1; i <= n0-1; i++) {
                        for(j = i+1; j <= n0; j++) {
                                edges_count = connect(i,j,edges_count,kn,edges);
                        }
                }

                //BAモデル成長過程//
                do {
                        //ノード追加
                        node_count++;

                        //m個接続先選択
                        for(i = 0; i < m; i++) {
                                do {
                                        //BA選択
                                        partner[i] = BA_select(kn,node_count);
                                        //重複選択の検知
                                        flag = 0;
                                        if(i > 0) {
                                                for(j = 0; j < i; j++) { if(partner[i] == partner[j]) { flag = 1; break; } }
                                        }
                                } while(flag == 1); //重複選択がある場合は再選択
                        }

                        //選択ノードに接続
                        for(i = 0; i < m; i++) { edges_count = connect(node_count,partner[i],edges_count,kn,edges); }
                } while(node_count < n);

                //ファイル出力//
                sprintf(s1,"ba%d.net",reiterate_count+1);
                edgesf = fopen(s1, "w");
                fprintf(edgesf,"*Vertices %d\n", node_count);
                fprintf(edgesf,"*Edges\n");
                for(i = 0; i < edges_count; i++) { fprintf(edgesf,"%d %d\n", edges[i][0],edges[i][1]); }
                fclose(edgesf);

                printf("done! %d\n",reiterate_count+1);

        }

        //メモリ開放
        free(kn);
        free(partner);
        for(i = 0; i < edges_max; i++) { free(edges[i]); }
        free(edges);

        time_end_all = clock();
        printf("execution time:%ld[ms]\n", (time_end_all-time_start_all));

        return 0;

}
