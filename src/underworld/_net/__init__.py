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

import urllib.request
import json
import underworld as uw

# add information about where the following GA parameters are defined
GA4_TRACKING_ID = "G-JQQHJRWD87"
API_SECRET = "cC7EhGFMQZaWgVsGUT5Zow"
GA_CLIENT_ID = uw._id


# Function to get user's IP address and geographic information
def get_geo_info():
    url = 'https://ipinfo.io/json'
    # try because connection to ipinfo ping may fail
    try:
        with urllib.request.urlopen(url) as response:
            if response.status == 200:
                data = response.json()
                return {
                    "country": data.get("country"),
                    "region": data.get("region"),
                    "city": data.get("city")
                }
    except:
        pass
    return None

def PostGA4Event( event_name, ev_params ):
    """ 
    Posts an Event Tracking message to Google Analytics.
    
    Current Underworld2 only dispatches a GA event when the underworld module
    is imported. This is effected by the calling of this function from
    underworld/__init__.py.
    
    Google Analytics uses the client id (GA4_CLIENT_ID) to determine unique users. In
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
    
    Full GA parameter reference may be found here:
    https://developers.google.com/analytics/devguides/collection/protocol/v1/parameters#ev
    
    Note, this function will return quietly on any errors. 
    
    Parameters
    ----------
    event_name: str
        Textual name for event_name. Can only contain alpha-numeric characters and underscores.
    ev_params: dict
        Optional paramerter dictionary for event.

    Add the following test here to ensure we're catching out when we're
    running from doctests to avoid dispatching metrics. 
    >>> print('Running in doctest? {}'.format(uw._in_doctest()))
    Running in doctest? True


    """
    geo_info = get_geo_info()

    try:
        url  = f"https://www.google-analytics.com/mp/collect?measurement_id={GA4_TRACKING_ID}&api_secret={API_SECRET}"

        payload = {
            "client_id": GA_CLIENT_ID,
            "events": [
                {
                    "name": event_name,
                    "params": {
                        "session_id": GA_CLIENT_ID,
                        "engagement_time_msec": 1,
                    },
                }
            ]
        }

        # add the input dict to the event params
        payload["events"][0]["params"].update(geo_info)
        payload["events"][0]["params"].update(ev_params)

        # convert data to btye string
        jsondata = json.dumps(payload)
        byte_data = jsondata.encode('utf-8')

        req = urllib.request.Request(url)
        req.add_header('Content-Type', 'application/json; charset=utf-8')
        req.add_header('Content-Length', len(byte_data))
        result = urllib.request.urlopen(req, byte_data)

        if result.status not in [200, 204]:
            print(f"Failed to post GA4 event. Status: {result.status}")

    except:
        pass
