# Tugas Jaringan Komputer
## Kelompok 13 K01

 - Faiz Ghifari Haznitrama  13515010
 - Yesa Surya               13515088
 - Dery Rahman Ahaddienata  13515097

### BAGIAN I - PETUNJUK PENGGUNAAN
1. Buka terminal, jalankan make
    ```sh
    $ make
    ```
2. Untuk membuat receiver, ketik
    ```sh
    $ ./recvfile <filename> <windowsize> <buffersize> <port>
    ```
    - <filename> : nama file output
    - <windowsize> : besar window size yang dipakai (RWS)
    - <buffersize> : besar buffer yang digunakan pada reveiver
    - <port> : port yang digunakan pada receiver
3. Untuk membuat sender, ketik
    ```sh
    $ ./sendfile <filename> <windowsize> <buffersize> <destination_ip> <destination_port>
    ```
    - <filename> merupakan nama file yang akan dikirim
    - <windowsize> : besar window size yang dipakai (SWS)
    - <buffersize> : besar buffer yang digunakan pada sender
    - <port> : port yang digunakan pada receiver

### BAGIAN II - CARA KERJA SLIDING WINDOW


### BAGIAN III - PERTANYAAN DAN JAWABAN
#### Pertanyaan
1. Apa yang terjadi jika advertised window yang dikirim bernilai 0? Apa cara untuk menangani hal tersebut?
2. Sebutkan field data yang terdapat TCP Header serta ukurannya, ilustrasikan, dan jelaskan kegunaan dari masing-masing field data tersebut!
#### Jawaban
1. Pada dasarnya, advertised window digunakan untuk memberitahukan jumlah window tersisa di receiver buffer kepada client sehingga data dalam segmen yang dikirimkan oleh client tidak melebihi jumlah window tersisa. Ketika advertised window bernilai 0, receiver buffer tidak memiliki ruang lagi untuk menampung segmen. Dengan demikian, koneksi TCP akan berhenti untuk sementara waktu yang artinya tidak akan ada lagi segmen yang dikirim oleh client ke server.
Salah satu penyebab permasalahan ini adalah aplikasi yang tidak begitu cepat dalam mengambil segmen yang telah ada di receiver buffer sehingga menyebabkan receiver buffer penuh. Yang dapat dilakukan untuk menangani hal tersebut adalah dengan memperkecil window size. Walaupun transfer data akan menjadi lebih lama karena akan ada lebih banyak waktu untuk menunggu ACK, cara ini memberikan waktu lebih kepada aplikasi untuk mengambil data dari receiver buffer.

2. ![alt text](tcp.png)
Ilustrasi TCP Header
*Skala diatas ilustrasi merupakan ukuran dari tiap fields*
    - SrcPort dan DstPort digunakan untuk mengidentifikasi port asal dan port tujuan pada koneksi TCP.
    - SequenceNum merupakan urutan data yang dikirimkan pada sebuah segmen.
    - Acknowledgement digunakan untuk memberitahukan kepada host lain bahwa segmen tertentu sudah diterima oleh receiver buffer.
    - AdvertisedWindow digunakan untuk memberitahukan kepada host lain jumlah window yang tersisa pada buffer.
    - Flags digunakan untuk memberikan informasi tertentu kepada host lain. Flags dapat diisi dengan nilai SYN, FIN, RESET, PUSH, URG, atau ACK.
    - Jika flag URG diset, maka UrgPtr merupakan pointer yang menunjukkan data yang tidak urgent.
    - Checksum digunakan untuk mendeteksi data corruption pada koneksi TCP.
    - HdrLen digunakan untuk memberitahu panjang dari TCP header atau mengukur panjang offset dari awal pake hingga awal data. HdrLen disebut juga Offset.
    - Options digunakan untuk menambahkan fitur baru atau 'addon', misalnya Window Scaling.
j. Data merupakan data sebenarnya yang dikirim melalui koneksi TCP.

### BAGIAN III - PEMBAGIAN TUGAS
- Faiz Ghifari Haznitrama 13515010   33%
Membuat struktur data segment dan beberapa API yang digunakan (membuat data menjadi segment, mengubah segment menjadi raw dan sebaliknya)
- Yesa Surya 13515088 33%
Membuat struktur data receiver buffer dan beberapa API yang digunakan (initializeBuffer, insertSegmentToBuffer, drainBuffer)
- Dery Rahman Ahaddienata 13515097 34%
Membuat mekanisme sliding window untuk pengiriman segmen
