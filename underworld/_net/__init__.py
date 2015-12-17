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

# The following should return the mac address as a machine identifier.
# This is useful to get an idea about unique users, but note that
# where no mac address is found, a random number is instead returned,
# but it's irrelevant as if there's no mac address, there's
# no network, so metrics will not be dispatched.
from uuid import getnode
GA_CLIENT_ID = str(getnode())


def PostGAEvent( category, action, label=None, value=None ):
    """ 
    Posts an Event Tracking message to Google Analytics.
    
    Full parameter reference may be found here:
    https://developers.google.com/analytics/devguides/collection/protocol/v1/parameters#ev
    
    Note, this function will return quietly on any errors. 
    
    Parameters
    ----------
    category: str
        Textual name for event category
    action: str
        Textual name for event action
    label: str (optional)
        Optional label for event
    value: non-negative integer (optional)
        Optional value for event
        

    """
    try:
        connection = httplib.HTTPSConnection('www.google-analytics.com')
        form_fields = {
        "v"  : "1",             # Version.
        "aip": "1",             # Enable IP anonymizing.
        "tid": GA_TRACKING_ID,  # Tracking ID / Web property / Property ID.
        "ds" : "app",           # Data Source.
        "cid": GA_CLIENT_ID,    # Anonymous Client ID.
        "t"  : "event",         # Event hit type.
        "an" : "underworld2",   # Application name.
        "av" : "alpha",         # Application version.
        "ec" : category,        # Event Category. Required.
        "ea" : action,          # Event Action. Required.
        "el" : label,           # Event label.
        "ev" : value,           # Event value.
        }
        params = urllib.urlencode(form_fields)
        connection.connect()
        connection.request('POST', '/collect?%s' % params, '', { "Content-Type": "application/x-www-form-urlencoded" })
    except:
        pass

