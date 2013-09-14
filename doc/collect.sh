#! /bin/bash

# Copyright (C) 2013 Ian Dunn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# IMPORTANT:
#
# This script assumes that the FTP_HOME directory is only used for
# proof files.  This is due to the fact that the password will be
# freely available, and thus might allow others to misuse this directory.

IFS=""

ARIS_DIR=""
LOG="$ARIS_DIR/doc/collect.log"
COLLECT_EL="$ARIS_DIR/doc/collect.el"
LAST_TIME=$(date +"%s")
CUR_TIME="$LAST_TIME"

HW_DIR="$ARIS_DIR/doc/proofs/problems"
MSG_SHENANIGANS="You have submitted an invalid file.\n\
 Perhaps this is only a mistake, but please DO NOT attempt \n\
to tamper with the files.  This means leaving the goals and \n\
premises alone, and only adding conclusions."
MSG_MODE="Your proof is in the wrong mode.  Perhaps it was \n\
supposed to be in boolean mode?"
ARIS_BIN="$ARIS_DIR/src/aris"

RM="shred -uz"

FTP_HOME=""
cd "$FTP_HOME"

if [ -z "$FTP_HOME" ] || [ -z "$ARIS_DIR" ]
then
    echo "You forgot to set FTP_HOME and ARIS_DIR."
    exit 99
fi

if [ ! -f "$LOG" ]
then
    touch "$LOG"
fi

function compare_files
{
    FILE="$1"
    HW="$2"
    TAG="$3"

    FIRST_GREP=$(grep -n "$TAG" "$FILE")
    if [ -z "$FIRST_GREP" ]
    then
	echo "$MSG_SHENANIGANS"
	return 33
    fi

    FILE_TAG_START=$(echo "$FIRST_GREP" | head -n 1 | cut -d':' -f 1)
    FILE_TAG_END=$(echo "$FIRST_GREP" | tail -n 1 | cut -d':' -f 1)
    FILE_TAG_DIFF=$(echo "$FILE_TAG_END - $FILE_TAG_START" | bc)

    FILE_TAGS=$(head -n $FILE_TAG_END "$FILE" | tail -n $FILE_TAG_DIFF)

    NEXT_GREP=$(grep -n "$TAG" "$HW")
    if [ -z "$NEXT_GREP" ]
    then
	echo "$MSG_SHENANIGANS"
	return 33
    fi

    HW_TAG_START=$(echo "$NEXT_GREP" | head -n 1 | cut -d':' -f 1)
    HW_TAG_END=$(echo "$NEXT_GREP" | tail -n 1 | cut -d':' -f 1)
    HW_TAG_DIFF=$(echo "$HW_TAG_END - $HW_TAG_START" | bc)

    HW_TAGS=$(head -n $HW_TAG_END "$HW" | tail -n $HW_TAG_DIFF)

    if [ "$HW_TAGS" != "$FILE_TAGS" ]
    then
	echo "$MSG_SHENANIGANS"
	return 33
    fi

    return 0
}

function rm_non
{
    for i in $(ls -A)
    do
        EXT=$(echo "$i" | cut -f 2 -d '.')
	if [ "$EXT" != "directive" ] && [ "$EXT" != "tle" ]
        then
            $RM "$i"
        fi
    done
}

while [ true ]
do
    CUR_TIME=$(date +"%s")
    DIF_TIME=$(echo "$CUR_TIME - $LAST_TIME" | bc)
    if [ $(( $DIF_TIME % 30 )) -eq 0 ]
    then
	rm_non
    fi
    if [ "$DIF_TIME" -ge 300 ]
    then
	for f in $(ls *.directive)
	do
	    DIRECTIVE_NAME="$f"
	    EMAIL=
	    INSTRUCTOR=
	    MSG=

	    EMAIL=$(grep "user" "$DIRECTIVE_NAME" | cut -f 2 -d ':' | cut -b 2-)
	    INSTRUCTOR=$(grep "instr" "$DIRECTIVE_NAME" | cut -f 2 -d ':' | cut -b 2-)

	    LOG_TIME=$(date +"%F %T")
	    echo "<$LOG_TIME> Found $f." >> "$LOG"

	    NUM_WORKS=$(echo "$(wc -l "$DIRECTIVE_NAME" | cut -f 1 -d' ') - 2" | bc)

	    echo "$NUM_WORKS"
	    PROC_STR="(send-aris-response \"$EMAIL\""

	    for i in $(seq 3 $(( $NUM_WORKS + 2 )))
	    do
		LINE=$(head -n $i "$DIRECTIVE_NAME" | tail -n 1)
		HW=$(echo "$LINE" | cut -f 1 -d '|')
		NAME=$(echo "$LINE" | cut -f 2 -d '|')

		if [ ! -f "$NAME" ] || [ -z "$HW" ]
		then
		    continue;
		fi

		HW_FILE="$HW_DIR/pf-$HW.tle"

		MSG+="Grade Report for Problem $HW:\n\n"

		ERR_MSG_T=""
		FILE_PROOF_LINE=$(grep '<proof ' "$NAME")
		HW_PROOF_LINE=$(grep '<proof ' "$HW_FILE")
		if [ "$FILE_PROOF_LINE" != "$HW_PROOF_LINE" ]
		then
		    ERR_MSG_T="$MSG_MODE"
		fi

		if [ ! -z "$ERR_MSG_T" ]
		then
		    MSG+="$ERR_MSG_T\n\n"
		    $RM "$NAME"
		    continue
		fi

		ERR_MSG_P=$(compare_files "$NAME" "$HW_FILE" "premises")
		if [ ! -z "$ERR_MSG_P" ]
		then
		    MSG+="$ERR_MSG_P\n\n"
		    $RM "$NAME"
		    continue
		fi

		ERR_MSG_G=$(compare_files "$NAME" "$HW_FILE" "goals")
		if [ ! -z "$ERR_MSG_G" ]
		then
		    MSG+="$ERR_MSG_G\n\n"
		    $RM "$NAME"
		    continue
		fi

		CUR_OUT="$($ARIS_BIN -e -g -f "$NAME")"
		MSG+="$CUR_OUT\n\n"
		$RM "$NAME"

	    done

	    echo "$MSG"
	    PROC_STR+=" \"$MSG\""
	    if [ ! -z $INSTRUCTOR ]
	    then
		PROC_STR+=" \"$INSTRUCTOR\""
	    fi
	    PROC_STR+=")"
	    echo "$PROC_STR"
	    emacs --batch -l "$COLLECT_EL" --eval "$PROC_STR"

	    $RM "$DIRECTIVE_NAME"
	done

	LAST_TIME="$CUR_TIME"
    fi
done
