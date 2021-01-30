import logging
import urllib
from _datetime import datetime as dt
import datetime
import configparser
import random
import tkinter
from tkinter import messagebox
import altair as alt
import altair_saver
import altair_viewer
#from altair_saver import save
import csv
import pandas as pd
import time
import os
from win10toast import ToastNotifier

starttime = dt.now()  # here the first time starting the program is saved

# set up a main function which will count down the minutes and run in the background
def main():
    # set up working directory
    # setup logger
    logging_level = "DEBUG"  # here you can change the logging level as needed, on file-srv logging level comes from the ini file
    # configuration for the logging file
    now = dt.now()
    target_file_logs = os.path.abspath(
        os.path.join(os.path.abspath(__file__), '..', 'log', 'datalog' + now.strftime("_%Y-%m-%d") + '.log'))
    if not os.path.exists(os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'log'))):
        os.makedirs(os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'log')))

    LOGFORMAT_LONG = '[%(asctime)s]\t{%(filename)s:%(lineno)d}\t%(levelname)s\t- %(message)s'
    logger = logging.getLogger(__name__)
    logging.basicConfig(level=logging_level, filename=target_file_logs, format=LOGFORMAT_LONG)
    logging.getLogger().setLevel(logging_level)
    logger.info("Logging is in action")

    # read in cinf file if we have one to get the access data for the data api
    config = configparser.ConfigParser()
    ini = os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'desktop.ini'))
    config.sections()
    config.read(ini)
    data_ip = config.get('BasicData', 'data_ip')
    consumption_max = config.get('BasicData', 'consumption_max')
    consumption_min = config.get('BasicData', 'consumption_min')
    date_end_time = config.get('BasicData', 'date_end_time')
    pom_session_time = config.get('BasicData', 'pom_session_time')
    pom_break_time = config.get('BasicData', 'pom_break_time')
    volt = config.get('BasicData', 'volt')
    electricity_price = config.get('BasicData', 'electricity_price')
    temp_min = config.get('BasicData', 'temp_min')
    temp_max = config.get('BasicData', 'temp_max')

    # counters for the push notifications are set
    ten_min_counter = datetime.timedelta(
        minutes=1)  # 5 minutes as a timer to set when the push notifications are allowed to be set
    push_bool_time = dt.now()

    # initial csv creating and writing headers
    filename = 'data' + now.strftime("_%Y-%m-%d") + '.csv'
    filepath = os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'output', 'data', filename))
    #with open(filepath, 'w') as data_file:  # for debug comment block
    #    writer = csv.writer(data_file)
    #    line = ['Date', 't_mean', 'Temperature', 'Humidity', 'heater_onOff', 'T_set', 'is_sitting',
    #            'T00_is_chair', 'T01_is_chair', 'T02_is_chair', 'PID Output', 'consumption']
    #    writer.writerow(line)

    # every 25 minutes 5 minutes timer, after 4 sessions 15 min timer
    timenow = starttime
    t_now = dt.now()  # Current time
    t_pom = int(pom_session_time) * 60  # Pomodoro session time
    t_delta = datetime.timedelta(0, t_pom)  # Timedelta of pomodoro session time
    t_fut = t_now + t_delta  # break start time
    delta_sec = int(pom_break_time) * 60  # break duration
    t_fin = t_now + datetime.timedelta(0, t_pom + delta_sec)  # End time of whole session

    t_graph = dt.strptime(date_end_time, '%H:%M')  # time at which the graph is drawn
    graph_boolean = 0  # graph boolean telling us, if the graph was already drawn for the dy

    root = tkinter.Tk()  # start messagebox
    root.withdraw()
    messagebox.showinfo("Pomodoro Started!",
                        "\nIt is now: " + timenow.strftime("%H:%M") + " hrs. \nTimer set fo 25 min.")
    root.update()

    n_pomodoros = 0  # number of pomodoro session
    n_breaks = 0  # if pomodoro is on break 1 or not 0
    n = 0  # normal counter to see

    while True:
        n = n + 1  # counting the intervals and calculating the timedelta in seconds
        push_timedelta = dt.now() - push_bool_time

        data, push_bool_time = get_data(data_ip, temp_min, temp_max, push_bool_time, ten_min_counter, push_timedelta,
                                        logger)
        push_bool_time = write_csv(data, volt, consumption_max, consumption_min, electricity_price, filepath,
                                   ten_min_counter, push_timedelta, push_bool_time, logger)

        # check, if the time is 16:00 and the graph booleans are 0, then graph function is triggered
        if graph_boolean == 0 and t_now >= t_graph:
            model_graph(filepath, logger)
            graph_boolean = 1

        if t_now < t_fut:
            logger.info('pomodoro Session in progress.')
        elif t_fut <= t_now <= t_fin:
            # check break boolean if break was already announced
            if n_breaks == 0:
                # get funny message to remind of break
                break_message = pom_push_messages()
                root = tkinter.Tk()  # starting the messagebox window
                root.withdraw()  # hiding them
                messagebox.showinfo("Pomodoro Break!", break_message)
                root.update()
                n_breaks += 1
        else:
            message = 'finished break. Please resume work, another pomodoro session starts now.'
            notification_station(message, logger)
            n_breaks = 0

            n_pomodoros += 1  # another pomodoro added on
            if n_pomodoros % 4 == 0:  # if it is the fourth pomodoro or a multple of 4
                delta_sec = 15 * 60  # break duration is longer
            else:
                delta_sec = 5 * 60  # else it is 5 min

            t_now = dt.now()
            t_fut = t_now + datetime.timedelta(0, t_pom)
            t_fin = t_now + datetime.timedelta(0, t_pom + delta_sec)

        time.sleep(5)  # reducing this to 5 seconds, because every five seconds we need to get data
        t_now = dt.now()


def get_data(data_ip, temp_min, temp_max, push_temp_time, ten_min_counter, push_timedelta, logger):
    f = urllib.request.urlopen(data_ip)    #open link
    value_read = f.read()

    value_read_str = str(value_read.decode("utf-8"))
    #debug option
    #value_read_str = '<!DOCTYPE HTML><html>Temperature[degC]: 30.50<br />Humidity[%]: 29.00<br />heater_onOff' \
     #               '[0:on,1:off]: 0<br />T_set: 20.00<br />is_sitting[0:true,1:false]: 0<br />T00_is_chair: 20.70<br ' \
      #              '/>T01_is_chair: 15.90<br />T02_is_chair: 20.80<br />PID Output: 10.00<br /></html>'

    value_read_str = value_read_str.replace('DOCTYPE', '')
    value_read_str = value_read_str.replace('html', '')
    value_read_str = value_read_str.replace('HTML', '')
    value_read_str = value_read_str.replace('<', '')
    value_read_str = value_read_str.replace('>', '')
    value_read_str = value_read_str.replace('!', '')
    value_read_str = value_read_str.replace('/', '')
    value_read_str = value_read_str.replace(' ', '')

    value_read_list = value_read_str.split("br")  # split into a list according to <br />

    # build data_dict
    data_dict = {}
    for i in value_read_list[:-1]:
        i = i.strip()
        # remove the units for clean keys
        first_bracket = i.find('[')
        last_bracket = i.find(']')
        unit = i[first_bracket:last_bracket]
        clean_i = i.replace(unit, '')
        clean_i = clean_i.replace('[', '')
        clean_i = clean_i.replace(']', '')
        ele_list = clean_i.split(':')
        key = ele_list[0]
        value = ele_list[1]
        data_dict[key] = value

    temp_mean = (float(data_dict['T00_is_chair']) + float(data_dict['T01_is_chair']) + float(
        data_dict['T02_is_chair'])) / 3
    data_dict['t_mean'] = temp_mean

    # checking the notifcation time restrictions
    if push_timedelta >= ten_min_counter:
        logger.info('checking for notifications')
        # get push messages out dependent on the consumption and temp field
        if data_dict['t_mean'] > float(temp_max) or data_dict['t_mean'] < float(temp_min):
            push_message = temp_push_messages(data_dict['t_mean'])
            notification_station(push_message, logger)
            logger.info(str(push_message))
            push_temp_time = dt.now()

    return data_dict, push_temp_time


def write_csv(new_data, volt, consumption_max, consumption_min, electricity_price, filepath, ten_min_counter,
              push_timedelta, push_temp_time, logger):
    logger.info('Writing data....')
    data = pd.read_csv(filepath)

    # calculating consumption
    seconds_duration = (dt.now() - starttime)
    # checking the notifcation time restrictions
    if push_timedelta >= ten_min_counter:
        if seconds_duration > datetime.timedelta(seconds=25):
            heating_on = data.loc[data['heater_onOff'] == 0]  # how long the heating was on
            t_heating = heating_on.shape[0] * 5  # time in seconds
            consumption = (t_heating * int(electricity_price) * int(
                volt) * 5 * 3600) / 1000  # missing baseline for arduino
            new_data['consumption'] = consumption

            if float(new_data['consumption']) > int(consumption_max) or float(new_data['consumption']) < int(
                    consumption_min):
                push_message = cons_push_messages(consumption)
                notification_station(push_message, logger)
                logger.info(str(push_message))
                push_temp_time = dt.now()
            # debug option
            # model_graph(filepath, logger)
    else:
        consumption = 0
        new_data['consumption'] = consumption

    line = [dt.strftime(dt.now(), '%d.%M.%Y %H:%M:%S'), new_data['t_mean'], new_data['Temperature'],
            new_data['Humidity'], new_data['heater_onOff'], new_data['T_set'], new_data['is_sitting'],
            new_data['T00_is_chair'], new_data['T01_is_chair'], new_data['T02_is_chair'], new_data['PIDOutput'],
            new_data['consumption']]

    with open(filepath, 'a') as data_file:
        writer = csv.writer(data_file)
        writer.writerow(line)

    return push_temp_time


def model_graph(filepath, logger):
    # build a diagram
    logger.info('Creating Overview diagram at the End of the day')
    logger.info('Drawing........')

    chart_filepath = os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'output', 'charts'))

    logger.info('Waiting for the elves to draw our graph')
    # getting data from the day before at the t_graph time
    data = pd.read_csv(filepath)

    # graph for temperature
    chart_temp = alt.Chart(data).mark_line().encode(
        x='Date',
        y='Temperature'
    )
    #altair_saver.save(chart_temp, os.path.join(chart_filepath, 'chart_temp' + starttime.strftime("_%Y-%m-%d") + '.png'))
    chart_temp.save(os.path.join(chart_filepath, 'chart_temp' + starttime.strftime("_%Y-%m-%d") + '.html'))


    chart_temp_2 = alt.Chart(data).mark_line().encode(
        x='Date',
        y='Temperature'
    ).interactive()
    chart_temp_2.show()

    # graph for Humidity
    chart_hum = alt.Chart(data).mark_line().encode(
        x='Date',
        y='Humidity:Q'
    )
    chart_hum.save(os.path.join(chart_filepath, 'chart_humidity' + starttime.strftime("_%Y-%m-%d") + '.html'))

    chart_hum_2 = alt.Chart(data).mark_line().encode(
        x='Date',
        y='Humidity:Q'
    ).interactive()
    chart_hum_2.show()

    logger.info('Using you private data to draw your privates.')

    # graph for consumption
    chart_cons = alt.Chart(data).mark_line().encode(
        x='Date',
        y='consumption'
    )
    chart_cons.save(os.path.join(chart_filepath, 'chart_consumption' + starttime.strftime("_%Y-%m-%d") + '.html'))

    chart_cons_2 = alt.Chart(data).mark_line().encode(
        x='Date',
        y='consumption'
    ).interactive()
    chart_cons_2.show()

    # graph for heater
    chart_heater = alt.Chart(data).mark_line().encode(
        x='Date',
        y='heater_onOff'
    )

    chart_heater_2 = alt.Chart(data).mark_line().encode(
        x='Date',
        y='heater_onOff'
    ).interactive()
    chart_heater_2.show()

    chart_heater.save(os.path.join(chart_filepath, 'chart_heater' + starttime.strftime("_%Y-%m-%d") + '.html'))
    logger.info('almost finished, need to get the eyes correctly.')


def pom_push_messages():
    # premessage so that the user knows why the message is there:
    pre_message = 'pomodoro time: '
    # here is a list of all possible push messages for pomodoro breaks
    push_message_list = ['Zeit für einen neuen Kaffee',
                         '🎵Steh auf jetzt oder nie, Girl, Zu viel geschlafen und das Leben ist halb vorbei🎶',
                         'Schon mal aus dem Fenster geschaut und die Sonne gesehen',
                         'Lebe gefährlich',
                         'finde heraus, ob du zu den 19 % Stolper und Stuzunfälle am Arbeitsplatz pro Jahr gehörst',
                         'Would the real slim Shady please stand up?', 'Beine bewegen, Arsch vertreten',
                         'Sitzen ist das neue Rauchen', 'Bewegung tut gut', 'Führe deine Figur mal spazieren',
                         'Bitte wenden', 'Gänge zum Kühlschrank verschlanken den Geist',
                         'Could the real slim Shady please stand up']

    # by creating random numbers between 0 and list length the push messages will be display in an arbitrarily order
    list_length = len(push_message_list)
    random_number = random.randrange(0, list_length)

    random_push_message = pre_message + push_message_list[random_number]

    return random_push_message


def cons_push_messages(consumption):
    push_message_list = ['Default, no more content for you.']
    # here is a list of all possible push messages for pomodoro breaks
    push_message_list_high_cons = ['Dir muss doch warm sein so wie du den Strom verheizt',
                                   'Bist du ein Reptil und brauchst die Wärme um dich Bewegen zu können?',
                                   'Es scheint zu ziehen, vielleicht schließt du lieber das Fenter anstatt zu heizen',
                                   "Baby it's cold outside...Schließ das Fenster!",
                                   'Wir heizen nicht für draußen',
                                   'Die Heizkosten sind zu hoch, das kannst du nicht mehr bezahlen',
                                   'Soviel Öl wie du verbrennst, kommt gleich die USA klopfen',
                                   'Mit deiner Einstellung kommt der Klimawandel schon im nächsten Jahr',
                                   'Dir muss doch warm sein so wie du den Strom verheizt',
                                   'Bist du ein Reptil und brauchst die Wärme um dich Bewegen zu können?',
                                   'Soviel Öl wie du verheizst kommt die USA gleich klopfen',
                                   'Das können wir uns nicht leisten',
                                   'Wir heizen nicht für draußen', 'Du bist kein Blockheizkraftwerk',
                                   'Ein Feuer wäre für die Temperatur effizienter', 'Zieh dir erst mal einen Pulli an',
                                   'Du solltest vielleicht lieber Home office machen',
                                   'Zu hohe Temperaturen senken die Fruchtbarkeit',
                                   'Wegen dir ist Tihange noch im Netz',
                                   'Wegen dir wird Hambi weggebaggert',
                                   'Es ist deine Schuld, dass die Welt ist wie sie ist',
                                   'Wem kalt ist, arbeitet zu langsam']

    push_message_list_low_cons = ['Du darfst es dir gemütlich machen', 'Gönn dir, Brudi', 'Mach den Gönnjamin',
                                  'Lieber Stoßlüften und Heizen, als Corona und sterben',
                                  'Du darfst die Jacke ausziehen', 'Wir sind doch nicht in der DDR',
                                  'Wir haben der Kaffeemaschine Bescheid gesagt',
                                  'Wahre Umweltschützer arbeiten im Kleinen', 'Ist dir nicht kalt?',
                                  'Heizt dein Nachbar für dich mit?',
                                  'Bist du ein Skandinavier gefangen in Deutschland?']

    # check if consumption is higher or lower than ???
    if consumption > 200:
        pre_message = 'High consumption: '
        push_message_list = push_message_list_high_cons
    elif consumption < 200:
        pre_message = 'Low consumption: '
        push_message_list = push_message_list_low_cons

    # by creating random numbers between 0 and list length the push messages will be display in an arbitrarily order
    list_length = len(push_message_list)
    random_number = random.randrange(0, list_length)

    random_push_message = pre_message + push_message_list[random_number]

    return random_push_message


def pir_push_messages():
    pre_message = 'PIR Sensor: '
    # here is a list of all possible push messages for pomodoro breaks
    push_message_list = ['AHA! Abstand, Hände waschen, Atemschutzmasken', 'BAH! Behelfsmaske, Abstand, Händewaschen!',
                         'Der Chef beobachtet dich', 'Kann das Meeting nicht auch eine Email sein?',
                         'Big brother is watching you', 'Hast du dich bewegt oder kommt da jemand?',
                         'Hör mal auf zu zappeln', 'Da hält sich jemand nicht an die Abstandsregeln',
                         'Corona kannst auch du bekommen', 'Achtung! Jemand nähert sich von hinten',
                         'AHA! AHA! AHA! AHA! Abstand Händewaschen Atemschutzmasken', 'Corona has entered the chat',
                         'Stay alert, the boss is looking', 'BAH! BAH! BAH! BAH! Behelfsmasken, Abstand, Händewaschen!',
                         'Du wirst beobachtet', 'Sneak attack!', 'Hast du dich bewegt oder war das hinter dir?']

    # by creating random numbers between 0 and list length the push messages will be display in an arbitrarily order
    list_length = len(push_message_list)
    random_number = random.randrange(0, list_length)

    random_push_message = pre_message + push_message_list[random_number]

    return random_push_message


def temp_push_messages(temp):
    push_message_list = []
    # here is a list of all possible push messages for pomodoro breaks
    push_message_list_high_temp = ['Brennt es oder ist es der Klimawandel?', 'Stoßlüften',
                                   'Selbst einem Kamel wäre es hier zu warm',
                                   'Hier würde selbst Prinz Andrew ins schwitzen kommen', 'Heiß heiß baby',
                                   'Zieh dir erst mal einen Pulli an, bevor du anfängst zu heizen',
                                   'Es ist heißer als der Belag von dem Pizzabaguette',
                                   'So kannst du dein Fett nicht schmelzen lassen',
                                   'Wenn du weiter so machst, kommt der Klimawandel schon nächstes Jahr',
                                   'Noch wärmer und du fängst an zu pfeifen', 'Das Bier wird zu schnell warm',
                                   'Gleich wirft der Hobbit den Ring hier rein',
                                   'Vielleicht lieber lüften anstatt zu heizen']

    push_message_list_low_temp = ['Es scheint zu ziehen, vielleicht schließt du lieber das Fenter anstatt,'
                                  'Bist du ein Skandinavier gefange in einem Deutschen Unternehmen?',
                                  'Der Kaffee wird zu schnell kalt',
                                  'Sind wir hier in der Arktis oder warum friert es?',
                                  'Ich kann den Eiskristallen am Fenster beim wachsen zu sehen',
                                  'Du darfst heizen', 'Zieh mal deine Jacke aus bevor du die Temperatur einstellst']

    # check if temperature is higher or lower than ???
    if temp > 30:
        pre_message = 'High temperature: '
        push_message_list = push_message_list_high_temp
    elif temp < 20:
        pre_message = 'Low temperature: '
        push_message_list = push_message_list_low_temp

    # by creating random numbers between 0 and list length the push messages will be display in an arbitrarily order
    list_length = len(push_message_list)
    random_number = random.randrange(0, list_length)

    random_push_message = pre_message + push_message_list[random_number]

    return random_push_message


def notification_station(push_message, logger):
    n = ToastNotifier()

    message_list = push_message.split(':')
    pre_message = message_list[0]
    push_message = message_list[1]
    icon_filepath = os.path.abspath(os.path.join(os.path.abspath(__file__), '..', 'icon', 'logo_free.ico'))

    n.show_toast(pre_message, push_message, duration=20, icon_path=icon_filepath)


def get_paths(config, conf_section, conf_str):
    # gets the paths from the ini file
    rel_path = config.get(conf_section, conf_str)
    conf_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), rel_path)
    return conf_path


if __name__ == '__main__':
    main()
