#!/usr/bin/env python
#
#   Copyright (C) 2021 Sean D'Epagnier
#
# This Program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.  

import os
import sys
from datetime import datetime

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from values import *

class ProbeData(object):
    pypilot_dir = os.getenv('HOME') + '/.pypilot/'
    calibration_filename = pypilot_dir + 'servocalibration'

    command_log_filename = pypilot_dir + 'servocommand.log'

    def __init__(self, client):
        self.client = client

        self.watch_values = {}
        self.watch_values['ap.enabled'] = False
        self.watch_values['ap.heading'] = 0
        self.watch_values['ap.heading_command'] = 0
        self.watch_values['ap.mode'] = ''

        self.watch_list = ['ap.enabled', 'ap.heading_command', 'ap.mode']

        for name in self.watch_list:
            self.client.watch(name)

    def send_info(self):
        self.driver.heading(self.watch_values['ap.heading'])
        self.driver.track(self.watch_values['ap.heading_command'])
        self.driver.mode(self.watch_values['ap.mode'])
        self.driver.endabled(1 if self.watch_values['ap.enabled'] else 0)

    def poll(self):
        self.client.watch('ap.heading', False if self.watch_values['ap.enabled'] else 1)
        self.client.watch('ap.heading_command', 1)

        msgs = self.client.receive()
        for name, value in msgs.items():
            self.watch_values[name] = value

        self.send_info()

def main():
    print('Probing Data')
    from server import pypilotServer
    server = pypilotServer()

    from client import pypilotClient
    client = pypilotClient(server)

    probe = ProbeData(client)

    period = .1
    lastt = time.monotonic()
    while True:
        probe.poll()
        client.poll()
        server.poll()

        dt = period - time.monotonic() + lastt
        if dt > 0 and dt < period:
            time.sleep(dt)
            lastt += period
        else:
            lastt = time.monotonic()


if __name__ == '__main__':
    main()
