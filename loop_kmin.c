//ベース　longループ
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "MT.h"

#define INF INT_MAX

int *get_mem_int1(int size){
        int *var=(int*)calloc(size,sizeof(int));
        if(var==NULL) { printf("mem over\n"); exit(1); }
        return var;
}

double *get_mem_double1(int size){
        double *var=(double*)calloc(size,sizeof(double));
        if(var==NULL) { printf("mem over\n"); exit(1); }
        return var;
}

int **get_mem_int2(int size1,int size2){
        int i;
        int **var=(int**)calloc(size1,sizeof(int*));
        if(var==NULL) { printf("mem over %d %d\n",size1,size2); exit(1); }
        for(i=0; i<size1; i++) {
                var[i]=(int*)calloc(size2,sizeof(int));
                if(var[i]==NULL) { printf("mem over %d %d\n",size1,size2); exit(1); }
        }
        return var;
}


//エッジ追加時のデータ更新
int connect_func(int a,int b,int node_size,int edges_size,int edges_count,int *kn,int **edges,int **ad){
        int i;
        //次数
        kn[a]++;
        kn[b]++;
        //エッジ
        edges[edges_count][0]=a;
        edges[edges_count][1]=b;
        edges_count++;

        if(a>node_size || b>node_size) { printf("error\n"); exit(1); }
        if(edges_count>edges_size) { printf("error\n"); exit(1); }

        //隣接リスト
        i=0;
        while(1) { if(ad[a][i]==0) { ad[a][i]=b; break; } i++; }
        i=0;
        while(1) { if(ad[b][i]==0) { ad[b][i]=a; break; } i++; }

        return edges_count;
}

//次数が最小のノードを選択
int kmin_select(int candidate_count,int *candidate,int *kn){
        int i;
        int kn_min;
        int selected_node;
        int k_candidate_count;

        int *k_candidate=get_mem_int1(candidate_count+1);

        kn_min=INF;
        for(i=0; i<candidate_count; i++) {
                if(kn_min>kn[ candidate[i] ]) {
                        kn_min=kn[ candidate[i] ];
                        k_candidate_count=0;
                        k_candidate[ k_candidate_count ]=candidate[i];
                }else if(kn_min==kn[ candidate[i] ]) {
                        k_candidate_count++;
                        k_candidate[ k_candidate_count ]=candidate[i];
                }
        }

        selected_node=k_candidate[ genrand_int32()%(k_candidate_count+1) ];

        free(k_candidate);
        return selected_node;
}

//lhop_kmin_search用
int partner_remove(int candidate_count,int *candidate,int *partner,int m){
        int i,j,k;
        for(i=0; i<candidate_count; i++) {
                for(j=0; j<m; j++) {
                        if(candidate[i]==partner[j]) {
                                for(k=i; k<candidate_count-1; k++) {
                                        candidate[k]=candidate[k+1];
                                }
                                candidate_count--;
                                i--;
                                break;
                        }
                }
        }
        return candidate_count;
}

//lホップ先にあるノードの中から次数が最小のノードを選択
int lhop_kmin_search(int start_node,int *partner,int node_size,int m,int hop,int **ad,int *kn){
        int i,j,k;
        int candidate_count;
        int node_count;
        int check_sum;
        int selected_node;

        int *candidate=get_mem_int1(node_size+1);
        int *node_set=get_mem_int1(node_size+1);
        int *check=get_mem_int1(node_size+1);

        candidate_count=1;
        candidate[candidate_count-1]=start_node;
        check[start_node]=1;

        for(i=0; i<hop; i++) {

                for(j=0; j<candidate_count; j++) { node_set[j]=candidate[j]; }
                node_count=candidate_count;

                //i+1ホップ先ノード集合を見つける
                candidate_count=0;
                for(j=0; j<node_count; j++) {
                        for(k=0; ad[ node_set[j] ][k]!=0; k++) {
                                if(check[ ad[ node_set[j] ][k] ]==0) {
                                        candidate[candidate_count]=ad[ node_set[j] ][k];
                                        candidate_count++;
                                        check[ ad[ node_set[j] ][k] ]=1;
                                }
                        }
                }

                //全ノードチェックでbreak
                check_sum=0;
                for(j=1; j<=node_size; j++) { check_sum+=check[j]; }
                if(check_sum==node_size) { break; }
        }

        //すでに選択されているノードを候補集合から除去
        candidate_count=partner_remove(candidate_count,candidate,partner,m);
        if(candidate_count>0) {
                //候補集合から次数が最小のノードを選択
                selected_node=kmin_select(candidate_count,candidate,kn);
        }else if(candidate_count==0) { //候補集合がすでに選択されているノードのみの場合 １ホップ前のノード集合から選ぶ
                node_count=partner_remove(node_count,node_set,partner,m);
                if(node_count==0) { printf("select error\n"); exit(1); }
                selected_node=kmin_select(node_count,node_set,kn);
        }else{ printf("select error\n"); exit(1); }

        free(candidate);
        free(node_set);
        free(check);

        return selected_node;
}

int main(void){
        init_genrand((unsigned)time(NULL));
        FILE *linkfs;
        int i,j,i2;
        int flag;
        int node_size;
        int N0;
        int m;
        int node_count;
        int edges_count;
        int edges_size;
        int data_count;
        int loop_size;
        int net_size;
        int net_count;
        char filename[20];

        printf("node size >>\n");
        scanf("%d",&node_size);
        if(node_size<=5) { printf("out of range   (node_size > 5)\n"); exit(1); }
        printf("loop size >>\n");
        scanf("%d",&loop_size);
        if(loop_size<=2) { printf("out of range   (loop_size > 2)\n"); exit(1); }
        printf("network size >>\n");
        scanf("%d",&net_size);
        if(net_size<=0) { printf("out of range   (net_size > 0)\n"); exit(1); }

        // node_size=5000;
        // loop_size=5;
        // net_size=10;

        N0=5; //初期ノード数
        m=4; //追加エッジ数
        edges_size=(node_size-N0)*m+10; //最終エッジ数

        int *partner=get_mem_int1(m);
        int **ad=get_mem_int2(node_size+1,node_size+1);
        int **edges=get_mem_int2(edges_size+1,2);
        int *kn=get_mem_int1(node_size+1);

        //ネットワーク生成ループ
        for(net_count=0; net_count<net_size; net_count++) {

                //初期化//
                node_count=N0;
                edges_count=0;
                for(i=0; i<edges_size+1; i++) { edges[i][0]=0; edges[i][1]=0; }
                for(i=0; i<node_size+1; i++) { kn[i]=0; for(j=0; j<node_size+1; j++) { ad[i][j]=0; } }

                //ファイルからネットワーク取得
                //edges_count=get_network(net_count,edges_count,kn,edges,edges_index,ad);

                //サイズがN0の完全グラフ定義
                for(i=1; i<N0; i++) {
                        for(j=i+1; j<=N0; j++) {
                                edges_count=connect_func(i,j,node_size,edges_size,edges_count,kn,edges,ad);
                        }
                }

                //ネットワーク成長ループ
                for(i=1; i<=node_size-N0; i++) {

                        //partner 接続先
                        for(j=0; j<m; j++) { partner[j]=0; }

                        for(j=0; j<m; j+=2 ) {
                                //接続先ランダム選択
                                do {
                                        partner[j]=genrand_int32()%node_count+1;
                                        flag=0;
                                        for(i2=0; i2<j; i2++) { if(partner[i2]==partner[j]) { flag=1; break; } }
                                } while(flag==1); //重複ノードはやり直し

                                //(loop_size-2)ホップ先ノード集合の中から次数が最小のノードを選択
                                partner[j+1]=lhop_kmin_search(partner[j],partner,node_count,m,loop_size-2,ad,kn);
                                for(i2=0; i2<j+1; i2++) { if(partner[i2]==partner[j+1]) { printf("partner error\n"); exit(1); } }
                        }

                        //接続
                        node_count++;
                        for(j=0; j<m; j++) { edges_count=connect_func(node_count,partner[j],node_size,edges_size,edges_count,kn,edges,ad); }

                }

                //リンク情報出力//
                sprintf(filename,"link%d.net",net_count+1);
                linkfs=fopen(filename,"w");
                fprintf(linkfs,"*Vertices %d\n",node_count);
                fprintf(linkfs,"*Edges\n");
                for(i=0; i<edges_count; i++) {
                        fprintf(linkfs,"%d %d\n",edges[i][0],edges[i][1]);
                }
                fclose(linkfs);

                printf("%d done!\n",net_count+1);
        }


        //メモリ開放
        free(kn);
        for(i=0; i<edges_size+1; i++) {  free(edges[i]); }
        free(edges);
        free(partner);
        for(i=0; i<node_size+1; i++) { free(ad[i]); }
        free(ad);

        return 0;
}
