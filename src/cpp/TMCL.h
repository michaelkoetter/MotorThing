#pragma once
#include <Stream.h>

// TODO this needs to be documented & cleaned up

#define TMCL_TELEGRAM_PAUSE_DEFAULT 5
#define TMCL_TELEGRAM_SIZE 9
#define TMCL_DOWNLOAD_SIZE 8

class TMCLTelegram
{
public:
  TMCLTelegram(char* buffer, unsigned int size);

  char              data(unsigned int position);
  void              data(unsigned int position, char data);

  int               intData(unsigned int position);
  void              intData(unsigned int position, int data);

  void              reset();
  unsigned int      size() { return m_size; }

  unsigned char     checksum() { return data(size()-1); }
  bool              checksumOK();
  void              updateChecksum() { data(size()-1, calculateChecksum()); };

  void              print(Stream& stream);

protected:

  unsigned char     calculateChecksum();

  char*             m_buffer;
  unsigned int      m_size;
};

class TMCLInterface
{
public:
  TMCLInterface(Stream* serial);

  void send(TMCLTelegram* telegram);
  bool receive(TMCLTelegram* telegram, int timeoutMillis = 1000);
  bool sendAndReceive(TMCLTelegram* telegram, int timeoutMillis = 1000);

  void setDebug(Stream* debug) { m_debug = debug; };

private:

  Stream*           m_serial;
  Stream*           m_debug;
};

class TMCLInstruction
{
public:
  TMCLInstruction(TMCLTelegram* telegram);

  unsigned char moduleAddress() { return m_telegram->data(0); }
  void moduleAddress(char value) { m_telegram->data(0, value); }

  unsigned char instruction() { return m_telegram->data(1); }
  void instruction(char value) { m_telegram->data(1, value); }

  unsigned char type() { return m_telegram->data(2); }
  void type(char value) { m_telegram->data(2, value); }

  unsigned char motor() { return m_telegram->data(3); }
  void motor(char value) { m_telegram->data(3, value); }

  int value() { return m_telegram->intData(4); }
  void value(int value) { m_telegram->intData(4, value); }

  void reset();

  void print(Stream& stream);
private:
  TMCLTelegram*     m_telegram;
};

class TMCLReply
{
public:
  TMCLReply(TMCLTelegram* telegram);

  unsigned char hostAddress() { return m_telegram->data(0); }
  unsigned char moduleAddress() { return m_telegram->data(1); }
  unsigned char status() { return m_telegram->data(2); }
  unsigned char instruction() { return m_telegram->data(3); }
  int value() { return m_telegram->intData(4); }

  void print(Stream& stream);
private:
  TMCLTelegram*     m_telegram;
};

class TMCLDownload
{
public:
  TMCLDownload(TMCLInterface* interface, TMCLTelegram* telegram);

  void begin(char address = 1);
  void end();
  void download(unsigned char* buf, unsigned int size);
  void download(Stream& stream);

  bool error() { return m_error; }

private:
  void            sendAndCheck(char status);

  char            m_address;
  TMCLInterface*  m_interface;
  TMCLTelegram*   m_telegram;
  bool            m_error;
  bool            m_downloading;
};
