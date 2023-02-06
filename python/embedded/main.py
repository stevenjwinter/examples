""" 
 This is an embedded script that uses the IO of the embedded controller 
 to retrieve GPS and Particle data and write to SD CARD

 It is still a work in progress and used only for development and testing.
 
"""

import os
import time

import adafruit_gps
import adafruit_pm25
import adafruit_sdcard
import board
import busio
import digitalio
import microcontroller
import neopixel
import storage
from analogio import AnalogIn
from digitalio import DigitalInOut, Direction, Pull
from microcontroller import Pin


# Set up logging
LOGGING_CONTROL_PIN = DigitalInOut(board.D8)     
LOGGING_CONTROL_PIN.direction = Direction.INPUT
LOGGING_CONTROL_PIN.pull = Pull.UP

# Grand Central default UART
UART = busio.UART(board.TX, board.RX, baudrate=9600)
I2C = busio.I2C(board.SCL, board.SDA)


# Initilialize globals
PARTICLE_25   = None
I2C_FOR_GPS = None

NEOPIXEL = neopixel.NeoPixel(board.NEOPIXEL, 1)
NEOPIXEL.brightness = 0.1

LAST_LATITUDE = None
LAST_LONGITUDE = None

def pixel_red():
    NEOPIXEL[0] = (255, 0, 0)
    NEOPIXEL.show()

def pixel_yellow():
    NEOPIXEL[0] = (255, 0, 0)
    NEOPIXEL.show()

def pixel_green():
    NEOPIXEL[0] = (0, 255, 0)
    NEOPIXEL.show()

def pixel_blue():
    NEOPIXEL[0] = (0, 0, 255)
    NEOPIXEL.show()

def pixel_black():
    NEOPIXEL[0] = (0, 0, 0)
    NEOPIXEL.show()

def initialize_gps():
    global I2C_FOR_GPS

    I2C_FOR_GPS = adafruit_gps.GPS_GtopI2C(I2C, debug=False) # Use I2C interface
    print('I2C gps initialized')

    I2C_FOR_GPS.send_command(b'PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0')
    I2C_FOR_GPS.send_command(b"PMTK220,2000")

    print('Done initializing GPS')
    time.sleep(2)


def read_gps(g):
    print('reading gps')

    ret = g.update()
    print('gps update(): ', ret)

    if not g.has_fix:
        # Try again if we don't have a fix yet.
        print("Waiting for fix...")
        data = g.read(64)  # read up to 32 bytes
        print(data)  # this is a bytearray type
        return
    else:
        print('has fix')

    if ret == False:
        clear_lcd()
        data = g.read(32)  # read up to 32 bytes
        print(data)  # this is a bytearray type
        lcd_write_string('GPS update() fail')
        print('gps update false, returning')
        return

    clear_lcd()
    set_cursor_position(0,0)        

    if not g.has_fix:
        print('no fix')
    else:
        print('Fix timestamp: {}/{}/{} {:02}:{:02}:{:02}'.format(
                g.timestamp_utc.tm_mon,   # Grab parts of the time from the
                g.timestamp_utc.tm_mday,  # struct_time object that holds
                g.timestamp_utc.tm_year,  # the fix time.  Note you might
                g.timestamp_utc.tm_hour,  # not get all data like year, day,
                g.timestamp_utc.tm_min,   # month!
                g.timestamp_utc.tm_sec))
        print('Latitude: {} degrees'.format(g.latitude))
        print('Longitude: {} degrees'.format(g.longitude))
        print('Fix quality: {}'.format(g.fix_quality))
        # Some attributes beyond latitude, longitude and timestamp are optional
        # and might not be present.  Check if they're None before trying to use!
        if g.satellites is not None:
            print('# satellites: {}'.format(g.satellites))
        if g.altitude_m is not None:
            print('Altitude: {} meters'.format(g.altitude_m))
        if g.track_angle_deg is not None:
            print('Speed: {} knots'.format(g.speed_knots))
        if g.track_angle_deg is not None:
            print('Track angle: {} degrees'.format(g.track_angle_deg))
        if g.horizontal_dilution is not None:
            print('Horizontal dilution: {}'.format(g.horizontal_dilution))

        lat = ("{0:.1f}".format(g.latitude))
        lon = ("{0:.1f}".format(g.longitude))
        s = "{}, {}".format(lat, lon)
        clear_lcd()
        print(s)
        #WriteString(s)
        #WriteString(' ')
    
    if g.satellites is not None:
        print("{} Sats".format(g.satellites))
        lcd_write_string(g.satellites)
    else:
        print('no satellites')
        lcd_write_string('0')
 



 
# Connect to the card and mount the filesystem.
def initialize_sd_card():
    spi = busio.SPI(board.SD_SCK, board.SD_MOSI, board.SD_MISO)
    cs = digitalio.DigitalInOut(board.SD_CS)
    sdcard = adafruit_sdcard.SDCard(spi, cs)
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")
    print('Done setting up SD Card')

def print_directory(path, tabs=0):
    for file in os.listdir(path):
        stats = os.stat(path + "/" + file)
        filesize = stats[6]
        isdir = stats[0] & 0x4000
 
        if filesize < 1000:
            sizestr = str(filesize) + " by"
        elif filesize < 1000000:
            sizestr = "%0.1f KB" % (filesize / 1000)
        else:
            sizestr = "%0.1f MB" % (filesize / 1000000)
 
        prettyprintname = ""
        for _ in range(tabs):
            prettyprintname += "   "
        prettyprintname += file
        if isdir:
            prettyprintname += "/"
        print('{0:<40} Size: {1:>10}'.format(prettyprintname, sizestr))
 
        # recursively print directory contents
        if isdir:
            print_directory(path + "/" + file, tabs + 1)

def initialize_particle_sensor():
    reset_pin = None
    global PARTICLE_25
    PARTICLE_25 = adafruit_pm25.PM25_UART(UART, reset_pin)

def lcd_command(b):
    UART.write( bytearray(b) )
    time.sleep(.01)

def lcd_write_string(text):
    lcd_command(text)

def clear_lcd():
    lcd_command([0xFE, 0x58])

def lcd_turn_on():
    lcd_command([0xFE, 0x42, 0x00]) 

def lcd_turn_off():
    lcd_command([0xFE, 0x46]) 

def set_cursor_position(col, row):  # col, row starting at 1
    lcd_command([0xFE, 0x47, col, row])

def backlight_white():
    lcd_command([0xFE, 0xD0, 0xFF, 0xFF, 0xFF])

def backlight_red():
    lcd_command([0xFE, 0xD0, 0xFF, 0x0, 0x0])

def backlight_yellow():
    lcd_command([0xFE, 0xD0, 0xFF, 0xFF, 0x0])

def backlight_green():
    lcd_command([0xFE, 0xD0, 0x0, 0x0F, 0x00])

def backlight_blue():
    lcd_command([0xFE, 0xD0, 0x0, 0x00, 0xFF])

def set_backlight_brightness(b):
    lcd_command([0xFE, 0x99, b])

def set_cursor_to_home_position():
    lcd_command([0xFE, 0x48])

def lcd_turn_backlight_on():
    lcd_command([0xFE, 0x42])

def lcd_write_string(text):
    ba = str(text)
    lcd_command(ba)

def lcd_set_contrast(c):
    lcd_command([0xFE, 0x50, c])

def print_air_quality_data(aqdata):
    print()
    print("Concentration Units (standard)")
    print("---------------------------------------")
    print(
        "PM 1.0: %d\tPM2.5: %d\tPM10: %d"
        % (aqdata["pm10 standard"], aqdata["pm25 standard"], aqdata["pm100 standard"])
    )
    print("Concentration Units (environmental)")
    print("---------------------------------------")
    print(
        "PM 1.0: %d\tPM2.5: %d\tPM10: %d"
        % (aqdata["pm10 env"], aqdata["pm25 env"], aqdata["pm100 env"])
    )
    print("---------------------------------------")
    print("Particles > 0.3um / 0.1L air:", aqdata["particles 03um"])
    print("Particles > 0.5um / 0.1L air:", aqdata["particles 05um"])
    print("Particles > 1.0um / 0.1L air:", aqdata["particles 10um"])
    print("Particles > 2.5um / 0.1L air:", aqdata["particles 25um"])
    print("Particles > 5.0um / 0.1L air:", aqdata["particles 50um"])
    print("Particles > 10 um / 0.1L air:", aqdata["particles 100um"])
    print("---------------------------------------")


def get_air_smoke(aqdata):
    #print(type(aqdata["particles 03um"]))
    #return aqdata["particles 03um"]
    return aqdata["pm100 standard"]


def log_data_to_sd_card():
    with open("/sd/temperature.txt", "a") as file:
        temperature = microcontroller.cpu.temperature
        print("Temperature = %0.1f" % temperature)
        file.write("%0.1f\n" % temperature)
    # File is saved
    time.sleep(1)


clear_lcd()
lcd_write_string('PM Logger')

pixel_red()

set_backlight_brightness(150)
lcd_set_contrast(150)
backlight_white()
lcd_turn_on()
print('Init PM25')
initialize_particle_sensor()
print('Done init pm25')

print(PARTICLE_25)

initialize_sd_card()
log_data_to_sd_card()
print('wrote to sd card')

initialize_gps()
pixel_green()

def log_info_to_sd_card(timestr, lat, lon, part):
    # add feedback (led?)
    with open("/sd/log.csv", "a") as file:
        #file.write("%0.1f\n" % temperature)
        file.write("{}, {}, {}, {} \n".format(timestr, lat, lon, part))
    # File is saved
    time.sleep(1)


def test_gps(gps):
    gps.update()
    # Every second print out current location details if there's a fix.
    if not gps.has_fix:
        # Try again if we don't have a fix yet.
        print("Waiting for fix...")
    else:
        # We have a fix! (gps.has_fix is true)
        # Print out details about the fix like location, date, etc.
        print("=" * 40)  # Print a separator line.
        print(
            "Fix timestamp: {}/{}/{} {:02}:{:02}:{:02}".format(
                gps.timestamp_utc.tm_mon,  # Grab parts of the time from the
                gps.timestamp_utc.tm_mday,  # struct_time object that holds
                gps.timestamp_utc.tm_year,  # the fix time.  Note you might
                gps.timestamp_utc.tm_hour,  # not get all data like year, day,
                gps.timestamp_utc.tm_min,  # month!
                gps.timestamp_utc.tm_sec,
            )
        )
        print("Latitude: {0:.6f} degrees".format(gps.latitude))
        print("Longitude: {0:.6f} degrees".format(gps.longitude))
        print("Fix quality: {}".format(gps.fix_quality))
        # Some attributes beyond latitude, longitude and timestamp are optional
        # and might not be present.  Check if they're None before trying to use!
        if gps.satellites is not None:
            print("# satellites: {}".format(gps.satellites))
        if gps.altitude_m is not None:
            print("Altitude: {} meters".format(gps.altitude_m))
        if gps.speed_knots is not None:
            print("Speed: {} knots".format(gps.speed_knots))
        if gps.horizontal_dilution is not None:
            print("Horizontal dilution: {}".format(gps.horizontal_dilution))
        if gps.height_geoid is not None:
            print("Height geo ID: {} meters".format(gps.height_geoid))




while True:

    pixel_blue()
    try:
        I2C_FOR_GPS.update()
        I2C_FOR_GPS.update()
        I2C_FOR_GPS.update()
        I2C_FOR_GPS.update()
    except:
        print('Error with GPS')
        pixel_red()
        continue

    pixel_green()
    try:
        read_gps(I2C_FOR_GPS)    
    except:
        print('error reading gps')
        clear_lcd()
        lcd_write_string('Error Read GPS')
        pixel_red()
        continue

    g = I2C_FOR_GPS

    if g is None or g.timestamp_utc is None:
        clear_lcd()
        print("Can't read gps object")
        lcd_write_string('No GPS or GPS time')
        pixel_red()
        continue
    
    if g.latitude is None or g.longitude is None:
        print("Can't read gps object")
        clear_lcd()
        print("Can't read gps position")
        lcd_write_string('No GPS position')
        pixel_red()
        continue

    if int(g.timestamp_utc.tm_mon)  == 0 or int(g.timestamp_utc.tm_mday) == 0 or int(g.timestamp_utc.tm_year) == 0 or int(g.timestamp_utc.tm_hour) == 0:
        print('Invalid time, skipping')
        clear_lcd()
        continue
    else:
        try:
            year =   int(g.timestamp_utc.tm_year)
            month =  int(g.timestamp_utc.tm_mon)
            day =    int(g.timestamp_utc.tm_mday)
            hour =   int(g.timestamp_utc.tm_hour)
            minute = int(g.timestamp_utc.tm_min)
        except:
            print("Can't read gps object")
            pixel_red()
            continue

    lcd_turn_on()
    pixel_green()
    timeString = "{:04d}/{:02d}/{:02d} {:02d}:{:02d}".format(year, month, day, hour, minute)
    showTimeString = "{:02d}:{:02d} utc".format(hour, minute)
    print(timeString)
    print('Latitude: {} degrees'.format(g.latitude))
    print('Longitude: {} degrees'.format(g.longitude))
    ltd = ("{0:.3f}".format(g.latitude))
    lng = ("{0:.3f}".format(g.longitude))

    set_cursor_position(1,1)
    lcd_write_string("GPS {} sats".format(g.satellites))
    set_cursor_position(1,2)
    lcd_write_string("{},{}".format(ltd, lng))
    time.sleep(1.0)

    set_cursor_position(1,1)
    lcd_write_string("                ");
    set_cursor_position(1,1)
    lcd_write_string(showTimeString)

    aqdata = PARTICLE_25.read()
    smoke = get_air_smoke(aqdata) 
    print("{},{},{},{}".format(timeString, ltd, lng, smoke))

    if LOGGING_CONTROL_PIN.value:
        print('NOT logging')
    else:  # Logging
        if LAST_LATITUDE is None:
            LAST_LATITUDE = ltd
            LAST_LONGITUDE = lng
            pixel_blue()
            print('logging first time')
            set_cursor_position(1,1)
            lcd_write_string('Writing 1st log ')
            log_info_to_sd_card(timeString, ltd, lng, smoke)
        elif LAST_LATITUDE is not None and LAST_LATITUDE != ltd and LAST_LONGITUDE != lng:
            pixel_blue()
            print('logging')
            set_cursor_position(1,1)
            lcd_write_string('Writing log...')
            log_info_to_sd_card(timeString, ltd, lng, smoke)
            LAST_LATITUDE = ltd
            LAST_LONGITUDE = lng
        else: # just skip same location
            print('not logging, same or missing')

    print('-----------')

    s = str('Particles: {}   '.format(smoke))  # sjw: clear one line?
    print(s)
    set_cursor_position(1,2)
    lcd_write_string(s)

    pixel_black()
    time.sleep(2.0)
    pixel_green()
