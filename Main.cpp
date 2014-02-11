// Alex Bertei e Lucas Pires

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


/*
|-------------------------------------------------------------------------------
| [STRUCT] Fat Regions
|-------------------------------------------------------------------------------
|
| Struct responsável por armazenar as informações dos cálculos realizados
| após a leitura do MBD
|
*/
typedef struct {



    int mbr_size;
    int reserved_region_size;
    int fat1_start;
    int fat_size;
    int fat2_start;;
    int root_directory_start;
    int root_directory_size;
    int data_region_start;
    int data_region_size;
    int cluster_size;
    int max_cluster;

} FatRegions;


/*
|-------------------------------------------------------------------------------
| [STRUCT] Fat 16 Boot Sector (MBR)
|-------------------------------------------------------------------------------
|
| Struct responsável por armazenar as informações do Boot Sector, o MBR. Essas
| que serão usadas ao longo do programa.
|
*/

typedef struct {

    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short;
    unsigned char media_descriptor;
    unsigned short sectors_per_fat;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_long;
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;

} __attribute((packed)) Fat16BootSector;


/*
|-------------------------------------------------------------------------------
| Variáveis Globais 
|-------------------------------------------------------------------------------
|
| Variáveis globais do programa que serão usados para diferentes funções do
| do programa.
|
*/

Fat16BootSector bs;
FatRegions fr;
unsigned short *fat1;
unsigned short *fat2;
unsigned short *cluster;
int fs_ID;
int i;
int j;
int ret;
FILE *in;


/*
|-------------------------------------------------------------------------------
| [FUNÇÃO] Lê MBR
|-------------------------------------------------------------------------------
|
| Função que realiza a leitura do MBR. 
|
|
*/
void leMBR() {

    // Se posiciona no início da partição
    fseek(in, 0x0, SEEK_SET);

    // Lê o MBR
    fread(&bs, sizeof(Fat16BootSector), 1, in);

    // Se a partição for maior que 32MB entra no if
    if (bs.total_sectors_short == 0) {

        fs_ID = 6;

    // Caso a partição seja menor que 32MB seta o fs_ID = 4
    } else {

        fs_ID = 4;

    }
}


/*
|-------------------------------------------------------------------------------
| [FUNÇÃO] Cálcula Regiões FAT
|-------------------------------------------------------------------------------
|
| É nesta função que são cálculadas a início e o tamanho de todas as regiões 
| da FAT que está sendo analisada.
|
*/
void calculaRegioesDaFat() {

    // Cálcula o início e o tamanho de cada região da FAT.
    fr.mbr_size = sizeof(Fat16BootSector);
    fr.reserved_region_size = fr.mbr_size + (bs.reserved_sectors - 1) * bs.sector_size;
    fr.fat1_start = fr.reserved_region_size;
    fr.fat_size = bs.sectors_per_fat * bs.sector_size;
    fr.fat2_start = fr.fat1_start + fr.fat_size;
    fr.root_directory_start = fr.fat2_start + fr.fat_size;
    fr.root_directory_size = bs.root_dir_entries * 32;
    fr.data_region_start = fr.root_directory_start + fr.root_directory_size;

    // Se a partição for maior que 32MB usa o total_sectors_long para o cálculo da data_region_size.
    if (fs_ID == 6) {

        fr.data_region_size = (bs.total_sectors_long * bs.sector_size) - (fr.reserved_region_size + (fr.fat_size * 2) + fr.root_directory_size); 

    // Se a partição for menor que 32MB usa o total_sectors_short para o cálculo da data_region_size.
    } else if (fs_ID == 4) {

        fr.data_region_size = (bs.total_sectors_short * bs.sector_size) - (fr.reserved_region_size + (fr.fat_size * 2) + fr.root_directory_size);

    }

    // Tamanho do cluster e número máximo de clusters.
    fr.cluster_size = bs.sector_size * bs.sectors_per_cluster;
    fr.max_cluster = (fr.data_region_size - fr.cluster_size) / fr.cluster_size;

}

/*
|-------------------------------------------------------------------------------
| [FUNÇÃO] Verifica FATs
|-------------------------------------------------------------------------------
|
| Função que compara as tabelas FAT #1 e a FAT #2 da partição passada.
|
|
*/
void funcaoVerificaFats() {

    printf("\n\n\n");
    
    // Percorre as 2 FATs.
    for (int i = 0; i < fr.fat_size; i++) {

        // Verificando se seus valores são diferentes.
        if (fat1[i] != fat2[i]) {

            printf("DIF %d:%d,%d\n", i, fat1[i], fat2[i]);

        }
    }
}




/*
|-------------------------------------------------------------------------------
| [FUNÇÃO PRINCIPAL] Main
|-------------------------------------------------------------------------------
|
| Função principal onde o programa é iniciado e recebe argumentos passados
| pelo usuário.
|
| 
*/
int main(int argc, char *argv[]) {
	
    
    printf("\n\n\n---------------------- INICIO PROGRAMA ----------------------\n\n\n");
    
    char *fl;
    int vf = 0; // Flag - Verificar as FATs.
    int bl = 0; // Flag - Imprimir lista de blocos livres.
    int bd = 0; // Flag - Imprimir lista de blocos livres com dados.
    int cf1 = 0; // Flag - copia o conteudo da segunda fat na primeira
    int cf2=0; // Flag - copia o conteudo da primeira fat na segunda
    
    // Verifica quais argumentos foram passados.
    if (argc > 1) {

        for (int i = 1; i < argc; i++) {
            
            // É para verificar as FATs?
            if (!strcmp(argv[i], "-vf")) {

                vf = 1;

            // É para mostrar os blocos livres?
            } else if (!strcmp(argv[i], "-bl")) {

                bl = 1;
            
            // É para mostrar os blocos livres com dados?
            } else if (!strcmp(argv[i], "-bd")) {

                bd = 1;

            //copia segunda fat para a primeira
            } else if (!strcmp(argv[i], "-cf1")) {

                cf1 = 1;
                
              // copia a primeira fat para a segunda
			}else if (!strcmp(argv[i], "-cf2")) {

                cf2 = 1;


            // Passou um arquivo FAT? Qual o nome do arquivo da FAT a ser lido?
            } else if (!strcmp(argv[i], "-fl")) {
                
                fl = argv[i + 1];
                
            // Ver as informações da FAT?
            }
        }
    }
    
    // Tenta abrir o arquivo da FAT passado pelo usuário   . 
    if ((in = fopen(fl, "rb")) == NULL) {

        printf("ERRO NA LEITURA DO ARQUIVO: Não foi possível abrir a imagem da FAT.");

    } else {

        // Faz a leitura do primeiro setor, MBR.
        leMBR();

        // Realiza o cálculo do inicio e do tamanho das regiões da FAT.
        calculaRegioesDaFat();
        
       
        
        // Aloca uma variável com o tamanho de uma fat e já lê a FAT #1
        fat1 = (unsigned short *) malloc(fr.fat_size);
        fread(fat1, fr.fat_size, 1, in);

        // Aloca uma variável com o tamanho de uma FAT e já lê a FAT #2
        fat2 = (unsigned short *) malloc(fr.fat_size);
        fread(fat2, fr.fat_size, 1, in);
        
        // Aloca uma variável com o tamanho de um cluster.
        cluster = (unsigned short *) malloc(fr.cluster_size);
        
        // Verifica as FATs?
        if (vf) { funcaoVerificaFats(); }

        
 }
 
}
