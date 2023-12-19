Implementazione dell'algoritmo k-means in più versioni:
   -k-means_sequential_AoS: k-means in versione sequenziale dove i punti sono memorizzati come un array di strutture;
   -k-means_sequential_SoA: k-means in versione sequenziale dove i punti sono memorizzati come una struttura di array;
   -k-means_parallel: k-means in versione parallela dove i punti sono memorizzati come una struttura di array.

Per ogni versione del k-means si può eseguire vari test modificando le seguenti costanti presenti nel codice:
   -DATASET_PATH: indica il percorso del dataset che si vuole utilizzare. I dataset utilizzabili sono quelli presenti nella cartella "datasets";
   -DESIRED_CONFIG: indica quale fra i set di centroidi presnti nel file config_sets.ini si vuole utilizzare;
   -ITERATION_NUMBER: numero di iterazioni desiderate per un'esecuzione di k-means.

Inoltre il file k_means sequential permette di modificare il numero di thread utilizzabili per un'esecuzione del k-means attraverso la variabile THREAD_NUMBER.
