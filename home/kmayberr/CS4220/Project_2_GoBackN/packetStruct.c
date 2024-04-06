struct packetStruct
{
  int type; // 1 for data, 2 for ACK
  int seq_no;
  int length;
  char data[512];
};