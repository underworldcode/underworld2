#!/bin/bash
#Script to write images or any other actions to be executed every dump timestep
#Assumes LavaVu installed in same dir as script, set LAVAVU_PATH to override
BASEDIR=$(dirname $0)
#Get first three known args and pass rest on as listed
OUTDIR=$1; shift
TIMESTEP=$1; shift
DBPATH=$1; shift

#CONFIG VARIABLES...NEED TO BE SET BEFORE RUNNING UNDERWORLD!!!!
#UWEMAIL=
#Host to connect to for results, if not set will use ip address of eth0
#UWHOST=
#Command to transfer files 
#ln to create symbolic links for files to local web server root
#scp for remote transfer etc
#UWSYNC="ln -s "
#Where to send files / create links with above command
#UWDEST=""

LAVAVU_PATH=${LAVAVU_PATH-$BASEDIR}

IDFILE=${OUTDIR}/id.txt

if [ ! -f ${IDFILE} ]; then
  #Generate 128-bit ID from un-blocking random number generator
  ID=$(xxd -l 16 -p /dev/urandom)
  echo ${ID} > ${IDFILE}

  #Create symlink for static content
  rm html
  ln -s ${LAVAVU_PATH}/html ${OUTDIR}

  #Launch viewer server in background
  #${LAVAVU_PATH}/LavaVu -h -p8080 "$@" ${DBPATH}&

  #Server sync enabled?
  if [ ! -z "$UWSYNC" ]; then
    #Get IP address if no host defined
    if [ -z "$UWHOST" ]; then
      UWHOST=$(printf "GET /plain HTTP/1.1\nHOST: ipecho.net\nBROWSER: web-kit\n\n" | nc ipecho.net 80 | sed -rn '/([0-9]{1,3}\.){3}[0-9]{1,3}/p' | sed 's/\r$//')
    fi

    printf "~~~~~~~~~~~~~~~ View Underworld results at: ~~~~~~~~~~~~~~~\nhttp://${UWHOST}/${ID}\n\n" > ${OUTDIR}/link.txt

    #Email notify
    if hash sendmail 2>/dev/null; then
      if [ ! -z "$UWEMAIL" ]; then
        #echo "Underworld job running, results available at: http://${UWHOST}/${ID}/" | mail -s "Underworld job commenced (${ID})" $UWEMAIL
        echo -e "subject: Underworld job commenced (${ID})\n\nUnderworld job running, results available at: http://${UWHOST}/${ID}/" | sendmail $UWEMAIL
      else
        echo "No email address provided (UWEMAIL)"
      fi
    else
      echo "sendmail not installed"
    fi

    #Send files to view location
    if [ ! -z "$UWSYNC" ]; then
      ${UWSYNC} $(readlink -f ${OUTDIR}) ${UWDEST}${ID}
    fi
  else
    echo "Output Sync disabled"
    printf "~~~~~~~~~~~~~~~ View Underworld results at: ~~~~~~~~~~~~~~~\nfile://$('pwd')/${OUTDIR}/index.html\n\n" > ${OUTDIR}/link.txt
  fi
else
  ID=$(cat ${IDFILE})
fi

#Dump images
CMD="${LAVAVU_PATH}/LavaVuOS -I -J -${TIMESTEP} ${@} ${DBPATH}"
echo ${CMD}
eval ${CMD}

#Run html.py if exists in LavaVu path
[ -f ${LAVAVU_PATH}/html.py ] && python ${LAVAVU_PATH}/html.py -d ${DBPATH} -u ${BASEDIR}/ -o ${OUTDIR} -j ${ID} > ${OUTDIR}/index.html

#Output link to results
if [ ! -z "$UWSYNC" ]; then
  cat ${OUTDIR}/link.txt
fi

