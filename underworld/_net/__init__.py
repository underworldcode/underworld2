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
import underworld as uw

GA_TRACKING_ID = "UA-35748138-7"
GA_CLIENT_ID = uw._id

def PostGAEvent( category, action, label=None, value=None ):
    """ 
    Posts an Event Tracking message to Google Analytics.
    
    Current Underworld2 only dispatches a GA event when the underworld module
    is imported. This is effected by the calling of this function from
    underworld/__init__.py.
    
    Google Analytics uses the client id (GA_CLIENT_ID) to determine unique users. In
    Underworld, we generate a random string for this id and record it in _uwid.py (the
    value is available via the uw._id attribute). If the file (_uwid.py) exists, it
    is not recreated, so generally it will only be created the first time you build
    underworld. As this is a 'per build' identifier, it means that all users of a
    particular docker image will be identified as the same GA user. Likewise, all
    users of a particular HPC Underworld module will also be identified as the same
    GA user. Note that users are able to set the UW_USER_ID environment variable
    which overrides the randomly generated string, though this is probably of limited
    use.
    
    Regarding HPC usage, it seems that the compute nodes on most machines are closed
    to external network access, and hence GA analytics will not be dispatched
    successfully. Unfortunately this means that most high proc count simulations
    will not be captured in GA data.
    
    GA also reports on number of 'sessions'. The single session in GA is considered
    to be usage where concurrent events occur within 30 minutes of each other. So if
    you `import underworld` 5 times every 29 minutes, it will count as a single session.
    However if you `import underworld` 5 times every 31 minutes it will count as 5
    sessions.
    
    Full GA parameter reference may be found here:
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


    Add the following test here to ensure we're catching out when we're
    running from doctests to avoid dispatching metrics. 
    >>> print('Running in doctest? {}'.format(uw._in_doctest()))
    Running in doctest? True


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

