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

bool TMCLInterface::receive(TMCLTelegram& telegram, int timeoutMillis)
{
  unsigned long timeout = millis() + timeoutMillis;
  int len = telegram.size();

  do {
    int available = m_serial->available();

    if (available >= len) {
      // we can read a complete telegram
      for (int i = 0; i < len; i++)
        telegram.data(i, m_serial->read());

      if (m_debug != 0) {
        m_debug->print("<< ");
        telegram.print(*m_debug);
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

void TMCLInterface::send(TMCLTelegram& telegram)
{
  telegram.updateChecksum();
  if (m_debug != 0) {
    m_debug->print(">> ");
    telegram.print(*m_debug);
    m_debug->print("\n");
  }

  for (int i = 0; i < telegram.size(); i++)
    m_serial->write(telegram.data(i));


  // if there is garbage on the wire, flush it now
  delay(1);
  m_serial->flush();
}

bool TMCLInterface::sendAndReceive(TMCLTelegram& telegram, int timeoutMillis)
{
  send(telegram);
  return receive(telegram, timeoutMillis);
}

TMCLInstruction::TMCLInstruction(TMCLTelegram* telegram)
  : m_telegram(telegram)
{

}

void TMCLInstruction::reset() {
  moduleAddress(1);
  instruction(0);
  type(0);
  motor(0);
  value(0);
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
