#!/bin/bash

# This program obtain all the tables with information about a certain column around a position from http://vizier.u-strasbg.fr/viz-bin/VizieR-2 
#
# Copyright (C) 2021 Adrian Hernandez Padron <adrianhdezp10@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#Colors to the help panel
greenColour="\e[0;32m\033[1m"
endColour="\033[0m\e[0m"
redColour="\e[0;31m\033[1m"
blueColour="\e[0;34m\033[1m"
yellowColour="\e[0;33m\033[1m"
purpleColour="\e[0;35m\033[1m"
turquoiseColour="\e[0;36m\033[1m"
grayColour="\e[0;37m\033[1m"

function helpPanel(){
	echo -e "\n${redColour}[!] Uso: ./vizierquery${endColour}"
	echo -e "\n\n\t This program works with two different syntax"
	echo -e "\n\n\t ---------------------------------------------------"
	echo -e "\n\n\t First:"
	echo -e "\n\n\t\t [-o][STR] Image .fits"
	echo -e "\n\t\t\t With this argument the program obtain the center and the radius from an image and works with that"
	echo -e "\n\n\t Second:"
	echo -e "\n\n\t\t [-m][FLT,FLT][J2000] With this option the center point is required "
	echo -e "\n\t\t\t Center of the image"
 	echo -e "\n\n\t\t [-r][FLT][arcmin] Radio"
    echo -e "\n\t\t\t Use this option if you want a circle region"
    echo -e "\n\n\t\t [-w][FLT[,FLT]][arcmin] Width"
    echo -e "\n\t\t\t Use this option if you want a box around the point"
	echo -e "\n\n\t ---------------------------------------------------"
	echo -e "\n\n\t [-c][STR] Desired column"
    echo -e "\n\t\t This follows the ucd syntax from vizier﻿"
	echo -e "\n\t\t [-ucd] With this command you can see all the ucd columns in your console, you can also grep then to find your desired colunmn name"
	echo -e "\n\n\t [-s] To download all the tables"
	echo -e "\n\n\t [-d] With this command you can search for a determinate word"
	echo -e "\n\n\t [-f][STR] Output format"
	echo -e "\n\t\t The requested output format can be the format supported by astquery(currently .txt or .fits)"
	echo -e "\n${redColour}[!] This program needs one of the two syntax([-o] or [-m] & [-r]//[-w]) and the column [-c] to work!${endColour}"
}

function ucd_column(){
	curl "http://vizier.u-strasbg.fr/viz-bin/asu-tsv?-source=METAucd"
}

function viziercurl(){
	if [ $column_counter -eq 1 ]; then
		columns_url="*$columns_input"
	fi
	if [ $radio_counter -eq 1 ]; then
		curl -o rawdata.txt --form -c=$center --form -c.rm="$radio" --form -ucd=$columns_input --form -source=$key_word "http://vizier.unistra.fr/viz-bin/asu-txt?-out=*pos.eq.ra;meta.main,*pos.eq.dec;meta.main,$columns_url"
		
	elif [ $width_counter -eq 1 ]; then
		curl -o rawdata.txt --form -c=$center --form -c.bm="$width" --form -ucd=$columns_input --form -source=$key_word "http://vizier.unistra.fr/viz-bin/asu-txt?-out=*pos.eq.ra;meta.main,*pos.eq.dec;meta.main,$columns_url"
	else
		centerra=$(astfits -h0 $overlapwith --skycoverage | grep "Center:" | awk '{print $2}')
		centerde=$(astfits -h0 $overlapwith --skycoverage | grep "Center:" | awk '{print $3}')
		rad1=$(astfits -h0 $overlapwith --skycoverage | grep "Width:" | awk '{print $2}')
		rad2=$(astfits -h0 $overlapwith --skycoverage | grep "Width:" | awk '{print $3}')
		curl -o rawdata.txt --form -c="$centerra $centerde" --form -c.bd="$rad1/2,$rad2/2" --form -ucd=$columns_input --form -source=$key_word "http://vizier.unistra.fr/viz-bin/asu-txt?-out=*pos.eq.ra;meta.main,*pos.eq.dec;meta.main,$columns_url"
	fi

	resource=$(cat rawdata.txt | grep -B 7 'pos.eq.ra' | grep '#RESOURCE')
	for NAME in $resource; do
		sed -n "/$NAME/,/^$/p" rawdata.txt >> "$key_word$columns_input"_catalogs.txt
	done 

	rm rawdata.txt
	if [ $download_parameter -eq 1 ]; then
	mkdir "$key_word$columns_input"_folder
	vizier_dataset=$(grep "#Table" -A1 "$key_word$columns_input"_catalogs.txt | awk '$1=="#Name:" {print $2}')
		for DATASET in $vizier_dataset; do
		if [ $column_counter -eq 1 ]; then
			columns=$(grep -w -A5 "$DATASET" "$key_word$columns_input"_catalogs.txt | grep -w -E "pos.eq.ra|pos.eq.dec|$columns_input" | awk '{print $1}')
		else
			columns=$(grep -w -A5 "$DATASET" "$key_word$columns_input"_catalogs.txt | grep -w -E "pos.eq.ra|pos.eq.dec" | awk '{print $1}')
		fi
			ast_columns=$(echo $columns | tr -s ' ' ',')
			if [ $(echo $ast_columns | tr -cd , | wc -c) -ge 1 ]
			then
				filename=$(echo $DATASET | tr -s '/' '_')
				echo $ast_columns
				if [ $width_counter -eq 1 ]; then
					astquery vizier -h0 --dataset=$DATASET --output="$filename"."$format" --center=$center --width=$width/60 -c$ast_columns 2>/dev/null
					mv "$filename"* "$key_word$columns_input"_folder
				elif [ $radio_counter -eq 1 ]; then
					astquery vizier -h0 --dataset=$DATASET --output="$filename"."$format" --center=$center --radius=$radio/60 -c $ast_columns 2>/dev/null 
					mv "$filename"* "$key_word$columns_input"_folder
				else
					astquery vizier -h0 --dataset=$DATASET --output="$filename"."$format" --overlapwith=$overlapwith -c $ast_columns 2>/dev/null 
					mv "$filename"* "$key_word$columns_input"_folder
				fi
			fi
		done
	fi
} 

format_counter=0; radio_counter=0; width_counter=0; ucd_parameter=0; download_parameter=0; column_counter=0;
parameter_counter=0; while getopts "o:m:r:w:c:ucd:f:s:d:h:" arg; do
	case $arg in
		o) overlapwith=$OPTARG; let parameter_counter+=2;;
		m) center=$OPTARG; let parameter_counter+=1;;
		r) radio=$OPTARG; let parameter_counter+=1; let radio_counter+=1;;
		w) width=$OPTARG; let parameter_counter+=1; let width_counter+=1;;
		c) columns_input=$OPTARG; let parameter_counter+=1; let column_counter+=1;;
		u) ucd_column; let ucd_parameter+=1;;
		f) format=$OPTARG; let format_counter+=1;;
		s) key_word=$OPTARG; let parameter_counter+=1;;
		d) download=$OPTARG; let download_parameter+=1;;
		h) helpPanel;;
	esac
done

if [ $format_counter -eq 0 ]
then
	format="fits"
fi

if [ $parameter_counter -eq 3 ] || [ $parameter_counter -eq 4 ] ; then
	viziercurl
else
	if [ $ucd_parameter -eq 1 ]; then
		ucd_column
	else
		helpPanel
	fi
fi