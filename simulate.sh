#!/bin/bash

SENARIO="20  40 \
         20  50 \
         10 100 \
         60  80 \
         30  40 \
          0  60 \
         50  20"

INTERVAL=1
SLEEPTIME=1
VALUE1=
VALUE2=

for VALUE in $SENARIO
do
        VALUE1=$VALUE2
        VALUE2=$VALUE

        if [ -z "$VALUE1" ]; then
                continue
        fi

        INC=$INTERVAL
        STAT="CHARGING "
        if [ "$VALUE1" -gt "$VALUE2" ]; then
                INC=-$INC
                STAT="CONSUMING"
        fi
        #echo "INC=$INC"
        echo "SCENARIO: BATTERY $STAT: LEVEL CHANGED FROM $VALUE1 TO $VALUE2"
        echo "  CHECK /proc/battery_test"
        while [ "$VALUE1" -ne "$VALUE2" ]
        do
                VALUE1=$(( $VALUE1 + $INC ))
                echo "$VALUE1" > /proc/battery_test
                sleep $SLEEPTIME
        done
done
