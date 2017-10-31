#!/data/data/u.r/usr/bin/python3.5
#encoding: utf-8

# Copyright (C) 2013 @XiErCh
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from datetime import datetime

import smbus


def _bcd_to_int(bcd):
    """Decode a 2x4bit BCD to a integer.
    """
    out = 0
    for d in (bcd >> 4, bcd):
        for p in (1, 2, 4 ,8):
            if d & 1:
                out += p
            d >>= 1
        out *= 10
    return int(out / 10)


def _int_to_bcd(n):
    """Encode a one or two digits number to the BCD.
    """
    bcd = 0
    for i in (n // 10, n % 10):
        for p in (8, 4, 2, 1):
            if i >= p:
                bcd += 1
                i -= p
            bcd <<= 1
    return bcd >> 1


class I2C_BUS(smbus.SMBus):
    def __init__(self,twi=0):
        self.__bus = twi
        super().__init__(twi)

    def __str__(self):
        return 'i2c-%s'% self.__bus

class DS1307():
    _REG_SECONDS = 0x00
    _REG_MINUTES = 0x01
    _REG_HOURS = 0x02
    _REG_DAY = 0x03
    _REG_DATE = 0x04
    _REG_MONTH = 0x05
    _REG_YEAR = 0x06
    _REG_CONTROL = 0x07


    def __init__(self, twi=0, addr=0x68):
        self._bus = I2C_BUS(twi)
        self._addr = addr


    def _write(self, register, data):
        self._bus.write_byte_data(self._addr, register, data)


    def _read(self, data):
        return self._bus.read_byte_data(self._addr, data)


    def _read_seconds(self):
        return _bcd_to_int(self._read(self._REG_SECONDS))


    def _read_minutes(self):
        return _bcd_to_int(self._read(self._REG_MINUTES))


    def _read_hours(self):
        d = self._read(self._REG_HOURS)
        if d & 0x40:
            return _bcd_to_int(d & 0x3F)
        else:
            h = _bcd_to_int(d & 0x1F)
            if d & 0x20:
                h += 11  # Convert 12h to 24h
            elif h == 12:
                h = 0
            return h


    def _read_day(self):
        return _bcd_to_int(self._read(self._REG_DAY))


    def _read_date(self):
        return _bcd_to_int(self._read(self._REG_DATE))


    def _read_month(self):
        return _bcd_to_int(self._read(self._REG_MONTH))


    def _read_year(self):
        return _bcd_to_int(self._read(self._REG_YEAR))


    def read_all(self):
        """Return a tuple such as (year, month, date, day, hours, minutes,
        seconds).
        """
        Y = self._read_year()
        M = self._read_month()
        D = self._read_date()
        d = self._read_day()
        h = self._read_hours()
        m = self._read_minutes()
        s = self._read_seconds()
        return (Y, M, D, d, h, m,s )


    def read_str(self):
        """Return a string such as 'YY-DD-MMTHH-MM-SS'.
        """
        Y = self._read_year()
        M = self._read_month()
        D = self._read_date()
        h = self._read_hours()
        m = self._read_minutes()
        s = self._read_seconds()
        return '%02d-%02d-%02dT%02d:%02d:%02d' % (Y, M, D, h, m, s)


    def read_datetime(self, century=21, tzinfo=None):
        """Return the datetime.datetime object.
        """
        Y = self._read_year()
        M = self._read_month()
        D = self._read_date()
        h = self._read_hours()
        m = self._read_minutes()
        s = self._read_seconds()

        return datetime( int(century - 1) * 100 + Y,M,D ,h ,m , s ,0, tzinfo=tzinfo)


    def write_all(self, seconds=None, minutes=None, hours=None, day=None,
            date=None, month=None, year=None, save_as_24h=True, century=21):
        """Direct write un-none value.
        Range: seconds [0,59], minutes [0,59], hours [0,23],
               day [0,7], date [1-31], month [1-12], year [0-99].
        """
        if seconds is not None:
            if seconds < 0 or seconds > 59:
                raise ValueError('Seconds is out of range [0,59].')
            self._write(self._REG_SECONDS, _int_to_bcd(seconds))

        if minutes is not None:
            if minutes < 0 or minutes > 59:
                raise ValueError('Minutes is out of range [0,59].')
            self._write(self._REG_MINUTES, _int_to_bcd(minutes))

        if hours is not None:
            if hours < 0 or hours > 23:
                raise ValueError('Hours is out of range [0,23].')
            if save_as_24h:
                self._write(self._REG_HOURS, _int_to_bcd(hours) | 0x40)
            else:
                if hours == 0:
                    h = _int_to_bcd(12) | 0x32
                elif hours <= 12:
                    h = _int_to_bcd(hours)
                else:
                    h = _int_to_bcd(hours - 12) | 0x32
                self._write(self._REG_HOURS, h)

        if year is not None:
            if year < 0: #or year > 99:
                raise ValueError('Years is out of range [0,99] [%s-%s].' % ( ( century -1 ) * 100, century * 100 ) )
            if year > (century-1)*100 :
                year = year - (century-1)*100

            self._write(self._REG_YEAR, _int_to_bcd(year))

        if month is not None:
            if month < 1 or month > 12:
                raise ValueError('Month is out of range [1,12].')
            self._write(self._REG_MONTH, _int_to_bcd(month))

        if date is not None:
            if date < 1 or date > 31:
                raise ValueError('Date is out of range [1,31].')
            self._write(self._REG_DATE, _int_to_bcd(date))

        if day is not None:
            if day < 1 or day > 7:
                raise ValueError('Day is out of range [1,7].')
            self._write(self._REG_DAY, _int_to_bcd(day))


    def write_datetime(self, dt):
        """Write from a datetime.datetime object.
        """
        self.write_all(dt.second, dt.minute, dt.hour,
                dt.isoweekday(), dt.day, dt.month, dt.year % 100)


    def write_now(self):
        """Equal to DS1307.write_datetime(datetime.datetime.now()).
        """
        self.write_datetime(datetime.now())


    def __str__(self):
        return "(RTC)%s:%s" % ( self._bus, hex(self._addr))


def main():
    import sys
    ds = DS1307(0, 0x68)

    if 'set' in sys.argv:
        import time as Time
        Y,M,D,h,m,s = Time.localtime( Time.time() )[:6]
        print('Setting date & time',Y, M, D, h, m, s)
        ds.write_all( seconds=s, minutes=m, hours=h, day=None, date=D, month=M, year=Y, save_as_24h=True)

    if 'sync' in sys.argv:
        print('Syncing from RTC-0',datetime.now())
        ds.write_now()


    print(ds ,ds.read_datetime())



if __name__ == '__main__':
    main()
