#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh


CMD=bin/vad  #write here the name and path of your program

if [[ ! -x $CMD ]]; then
   echo "Error. Program not found (edit this script)"
   echo "CMD: $CMD"
   exit 1
fi

if [[ $# == 0 ]]; then
    echo $0 file1.wav [file2.wav ...]
    exit 1
fi


for filewav in $*; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}

    $CMD -i $filewav -o $filevad || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD $filewav $filevad $filewavOut || exit 1

done

exit 0
