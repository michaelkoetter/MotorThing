#include <Arduino.h>
#include "TMCL.h"

TMCLTelegram::TMCLTelegram(char* buffer, unsigned int size)
  : m_buffer(buffer), m_size(size)
{

}

char TMCLTelegram::data(unsigned int position) {
  if (position < m_size) return m_buffer[position];
  return -1;
}

void TMCLTelegram::data(unsigned int position, char data) {
  if (position < m_size) m_buffer[position] = data;
}

int TMCLTelegram::intData(unsigned int position) {
  if (position < m_size-4) {
    return (int) m_buffer[position++] << 24
      | m_buffer[position++] << 16
      | m_buffer[position++] << 8
      | m_buffer[position];
  }

  return 0;
}

void TMCLTelegram::intData(unsigned int position, int data) {
  if (position < m_size-4) {
    m_buffer[position++] = data >> 24;
    m_buffer[position++] = data >> 16;
    m_buffer[position++] = data >> 8;
    m_buffer[position] = data & 0xFF;
  }
}

void TMCLTelegram::reset() {
  memset(m_buffer, 0, m_size);
}

void TMCLTelegram::print(Stream& stream)
{
  for (int i = 0; i < m_size; i++) {
    stream.printf(" %#02x", m_buffer[i]);
  }

  if (checksumOK()) {
    stream.print(" CRC: OK");
  } else {
    stream.print(" CRC: ERROR");
  }
}

bool TMCLTelegram::checksumOK() {
  return checksum() == calculateChecksum();
}

unsigned char TMCLTelegram::calculateChecksum() {
  unsigned char _checksum = 0;
  for (int i = 0; i < m_size - 1; i++) {
    _checksum += m_buffer[i];
  }
  return _checksum;
}

TMCLInterface::TMCLInterface(Stream* serial)
  : m_serial(serial), m_debug(0)
{

}

bool TMCLInterface::receive(TMCLTelegram* telegram, int timeoutMillis)
{
  unsigned long timeout = millis() + timeoutMillis;
  int len = telegram->size();

  do {
    int available = m_serial->available();

    if (available >= len) {
      // we can read a complete telegram
      for (int i = 0; i < len; i++)
        telegram->data(i, m_serial->read());

      if (m_debug != 0) {
        m_debug->print("<< ");
        telegram->print(*m_debug);
        m_debug->print("\n");
      }
      return true;
    } else if (available > 0) {
      // telegram is not complete yet
    }

    yield();
  } while (millis() < timeout);

  // timeout
  return false;
}

void TMCLInterface::send(TMCLTelegram* telegram)
{
  telegram->updateChecksum();
  if (m_debug != 0) {
    m_debug->print(">> ");
    telegram->print(*m_debug);
    m_debug->print("\n");
  }

  for (int i = 0; i < telegram->size(); i++)
    m_serial->write(telegram->data(i));


  // if there is garbage on the wire, flush it now
  delay(1);
  m_serial->flush();
}

bool TMCLInterface::sendAndReceive(TMCLTelegram* telegram, int timeoutMillis)
{
  send(telegram);
  return receive(telegram, timeoutMillis);
}

TMCLInstruction::TMCLInstruction(TMCLTelegram* telegram)
  : m_telegram(telegram)
{

}

void TMCLInstruction::reset() {
  m_telegram->reset();
  moduleAddress(1);
}

void TMCLInstruction::print(Stream& stream)
{
  stream.printf("INSTRUCTION ->%d instruction: %d type: %d motor: %d value: %d (CRC %s)\n",
    moduleAddress(), instruction(), type(), motor(), value(),
    m_telegram->checksumOK() ? "OK" : "ERROR");
}


TMCLReply::TMCLReply(TMCLTelegram* telegram)
  : m_telegram(telegram)
{

}

void TMCLReply::print(Stream& stream)
{
  stream.printf("REPLY %d->%d status: %d instruction: %d value: %d (CRC %s)\n",
    moduleAddress(), hostAddress(), status(), instruction(), value(),
    m_telegram->checksumOK() ? "OK" : "ERROR");
}

TMCLDownload::TMCLDownload(TMCLInterface* interface, TMCLTelegram* telegram)
  : m_interface(interface), m_telegram(telegram), m_error(false), m_downloading(false)
{

}

void TMCLDownload::sendAndCheck(char status)
{
  if (m_interface->sendAndReceive(m_telegram)) {
    TMCLReply reply(m_telegram);
    if (!m_telegram->checksumOK() || reply.status() != status) {
      // TMCL error
      m_error = true;
    }
  } else {
    // timeout
    m_error = true;
  }
}


void TMCLDownload::begin()
{
  if (!m_downloading) {
    m_downloading = true;
    m_error = false;

    TMCLInstruction startDownload(m_telegram);
    startDownload.reset();
    startDownload.instruction(132);
    sendAndCheck(100);
  }
}

void TMCLDownload::end()
{
  if (m_downloading) {
    m_downloading = false;

    TMCLInstruction stopDownload(m_telegram);
    // send a "NOOP"
    stopDownload.reset();
    sendAndCheck(101);

    // send the actual stop instruction
    stopDownload.reset();
    stopDownload.instruction(133);
    sendAndCheck(100);
  }
}

void TMCLDownload::download(unsigned char* buf, unsigned int size)
{
  if (m_error || !m_downloading) return;  // can't continue

  // The binary download format is 8 bytes per instruction, it doesn't contain
  // the module address (first byte)

  int telegramPos = 0;
  for (int bufPos = 0; bufPos < size; bufPos++) {
    if (m_error) break;

    if (telegramPos == 0) {
      m_telegram->reset();
      telegramPos++;
    }

    if (telegramPos < m_telegram->size()) {
      m_telegram->data(telegramPos++, buf[bufPos]);

      if (telegramPos >= m_telegram->size()) {
        Serial.print("DOWNLOAD");
        m_telegram->print(Serial);
        Serial.print("\n");

        if (m_telegram->checksumOK()) {
          // finished - add address and send!
          m_telegram->data(0, 1);
          m_telegram->updateChecksum();
          telegramPos = 0;
          sendAndCheck(101);
          yield();
        } else {
          // checksum error in download data
          m_error = true;
        }
      }
    }
  }
}
