#!/bin/bash

ICON_SIZES="16 22 24 32 48 64 128 256 512";

resize_icon() {
    if [ -e $1.png ]; then
        for i in $ICON_SIZES; do
            printf "Converting $1.png to %10s px... " "${i}x${i}";
            convert $1.png -resize ${i}x${i} ${1}_${i}x${i}.png;
            printf "DONE.\n";
        done;
    else
        echo "resize_icon: file $1.png does not exist";
    fi;
}

clean_icons() {
    for i in $ICON_SIZES; do
        local icon_filename="${1}_${i}x${i}.png"
        if [ -e $icon_filename ]; then
            rm -f $icon_filename;
        fi;
    done;
}

resize_icon icon_simple;
resize_icon icon_round1;
