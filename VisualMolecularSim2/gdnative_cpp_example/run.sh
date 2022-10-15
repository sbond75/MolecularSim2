ext="$(realpath ../..)"
# Don't put more than about one of these onto the PYTHONPATH, so just check the end of it for simplicity:
if [[ $PYTHONPATH != *":$ext" ]]; then # https://reactgo.com/bash-check-string-ends-with-other/
    export PYTHONPATH="$PYTHONPATH:$ext"
fi

pushd .
cd ../..

prog="/Applications/Godot_mono.app/Contents/MacOS/Godot"
args="--path ./VisualMolecularSim2"
if [ -z "$noLLDB" ]; then # Can provide `noLLDB=1 bash run.sh` on the command line to run without LLDB.
    if [ -z "$noRun" ]; then # Can provide `noRun=1 bash run.sh` on the command line to run without starting the debugged program right away.
	run="-o run"
    else
	run=
    fi
    cmd="lldb $run $prog -- $args"
else
    cmd="$prog $args"
fi
if [ -z "$IN_NIX_SHELL" ]; then
    nix-shell shell.nix --command "$cmd"
else
    $cmd
fi

popd
