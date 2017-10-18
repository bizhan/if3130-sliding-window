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
Pada program kami, struktur BufferArray digunakan untuk merepresentasikan sender atau receiver buffer. Pada sisi client, setiap data yang dibaca kemudian dienkapsulasi menjadi sebuah segment dengan fungsi createSegment untuk kemudian dimasukkan ke sender buffer. Program akan terus membaca data hingga sender buffer penuh.

Data kemudian dikirimkan menuju receiver buffer. Setiap segment dari sender buffer akan dikirim sejumlah window sizenya pada sender (SWS), segment tersebut jika telah diterima oleh receiver, receiver akan mengirim ACK bahwa segment tersebut telah diterima. Tetapi sebelum itu, receiver melakukan pengecekan error menggunakan checksum untuk memastikan segment yang diterima benar dan tidak mengandung error, barulah setelah itu ACK dikirim menuju sender. Ketika sender menerima ACK, sender akan mengetahui segment mana yang belum diterima oleh receiver. LAR pada sender menunjukan segment yang dipastikan telah diterima oleh receiver sehingga ketika sender menerima sebuah ACK window sender akan bergeser ke nextSeqNum yang ditunjukan oleh ACK tersebut. Hal ini terus dilakukan hingga receiver buffer penuh.

Ketika receiver buffer telah penuh, window size mengecil dan program mulai memindahkan data dari receiver buffer ke filesystem dengan fungsi drainBuffer. Fungsi ini akan mengosongkan receiver buffer dan menuliskan keluaran pada filename yang telah dispesifikasikan. Selain dipanggil pada saat receiver buffer penuh, drainBuffer juga akan dipanggil ketika data yang diterima oleh receiver buffer merupakan sentinel yang menandakan akhir dari sebuah file yang dikirim. Pada sender, sender mengetahui receiver buffer telah penuh ketika menerima sebuah ACK yang mengandung Advertise Window Size sebesar 0. Dari sini, sender akan melakukan pengosongan pada sender buffer dan mengisi kembali sender buffer dengan segment-segment yang belum terkirim dan masih tersimpan di filesystem. Setelah sender buffer kembali penuh dan receiver buffer telah kosong, proses pengiriman file akan berlanjut kembali hingga seluruh file selesai dikirimkan yang ditandai dengan sebuah sentinel yang diterima oleh receiver buffer.

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
- Faiz Ghifari Haznitrama 13515010   32%
Membuat struktur data segment dan beberapa API yang digunakan (membuat data menjadi segment, mengubah segment menjadi raw dan sebaliknya)
- Yesa Surya 13515088 32%
Membuat struktur data receiver buffer dan beberapa API yang digunakan (initializeBuffer, insertSegmentToBuffer, drainBuffer)
- Dery Rahman Ahaddienata 13515097 36%
Implementasi sliding window untuk pengiriman segmen. Membuat socket UDP.