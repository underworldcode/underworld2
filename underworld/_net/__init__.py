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

import json,urllib
import http.client
import urllib
import uuid
import underworld as uw

GA_TRACKING_ID = "UA-35748138-7"
GA_CLIENT_ID = uw._id

def PostGAEvent( category, action, label=None, value=None ):
    """ 
    Posts an Event Tracking message to Google Analytics.
    
    Full parameter reference may be found here:
    https://developers.google.com/analytics/devguides/collection/protocol/v1/parameters#ev
    
    Note, this function will return quietly on any errors. 
    
    Parameters
    ----------
    category: str
        Textual name for event category.
    action: str
        Textual name for event action.
    label: str
        Optional label for event.
    value: non-negative integer
        Optional value for event.
        

    """
    try:
        connection = http.client.HTTPSConnection('www.google-analytics.com')
        form_fields = {
        "v"  : "1",             # Version.
        "aip": "1",             # Enable IP anonymizing.
        "tid": GA_TRACKING_ID,  # Tracking ID / Web property / Property ID.
        "ds" : "app",           # Data Source.
        "cid": GA_CLIENT_ID,    # Anonymous Client ID.
        "t"  : "event",         # Event hit type.
        "an" : "underworld2",   # Application name.
        "av" : uw.__version__,  # Application version.
        "ec" : category,        # Event Category. Required.
        "ea" : action,          # Event Action. Required.
        "el" : label,           # Event label.
        "ev" : value,           # Event value.
        "cm2": uw.nProcs(),     # Number of processes used. Stored into custom metric 2. 
        "cd5": str(uw.nProcs()),# Number of processes used. Stored into custom dim 2. Not sure if necessary.
        }
        import os
        # add user id if set
        if "UW_USER_ID" in os.environ:
            form_fields["uid"] = os.environ["UW_USER_ID"]
            form_fields["cd4"] = os.environ["UW_USER_ID"]
        
        if "UW_MACHINE" in os.environ:
            form_fields["cd6"] = os.environ["UW_MACHINE"]

        params = urllib.urlencode(form_fields)
        connection.connect()
        connection.request('POST', '/collect?%s' % params, '', { "Content-Type": "application/x-www-form-urlencoded" })
    except:
        pass

