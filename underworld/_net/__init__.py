##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module implements some basic functionality for posting usage metrics
to google analytics.
"""

import json,httplib,urllib
import httplib
import urllib
import uuid

GA_TRACKING_ID = "UA-35748138-7"
# generate random uuid per import
GA_CLIENT_ID = str(uuid.uuid4())


def PostGAEvent( category, action, label=None, value=None ):
    connection = httplib.HTTPSConnection('www.google-analytics.com')
    """ Posts an Event Tracking message to Google Analytics. """
    form_fields = {
    "v"  : "1",             # Version.
    "aip": "1",             # Enable IP anonymizing
    "tid": GA_TRACKING_ID,  # Tracking ID / Web property / Property ID.
    "cid": GA_CLIENT_ID,    # Anonymous Client ID.
    "t"  : "event",         # Event hit type.
    "ec" : category,        # Event Category. Required.
    "ea" : action,          # Event Action. Required.
    "el" : label,           # Event label.
    "ev" : value,           # Event value.
    }
    params = urllib.urlencode(form_fields)
    connection.connect()
    connection.request('POST', '/collect?%s' % params, '', { "Content-Type": "application/x-www-form-urlencoded" })

