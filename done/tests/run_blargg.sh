#!/bin/sh

prog="$(basename "$0")"
temp=/tmp/"${prog}-$$-stdout.txt"
temp2=/tmp/"${prog}-$$-stderr.txt"

trap 'err=$?;/bin/rm -f "${temp} ${temp2}" >/dev/null 2>&1; exit $err' 0 1 2 3 15

# ======================================================================
esc="\033["
end="${esc}0m"
red="${esc}31m"
green="${esc}32m"

# ======================================================================
# reports messages to stderr and exists
error () {
    echo $@ 1>&2
    exit 1
}

# ======================================================================
# reports messages to stderr and exists
display () {
    output="$(cat "$1")"
    if [ "x$output" = 'x' ]; then
        echo '    empty output. Did you forget to compile with -DBLARGG ?'
    else
        echo "$output"
    fi
}

# ======================================================================
rootdir="$(realpath "$(dirname "$(realpath "$0")")/..")"
exec="${rootdir}/test-gameboy"
[ -x "${exec}" ] || error "Cannot find \"${exec}\""

testdir="${rootdir}/tests/data/blargg_roms"
[ -d "${testdir}" ] || error "Cannot find \"${testdir}\" subdirectory"

# ======================================================================
for item in \
    01-special.gb:5            \
    02-interrupts.gb:5         \
    "03-op sp,hl.gb":5         \
    "04-op r,imm.gb":7         \
    "05-op rp.gb":7            \
    "06-ld r,r.gb":5           \
    07-jr,jp,call,ret,rst.gb:4 \
    "08-misc instrs.gb":5      \
    "09-op r,r.gb":15          \
    "10-bit ops.gb":20         \
    "11-op a,(hl).gb":25       \
    instr_timing.gb:5
do
    gb_file=$(echo $item | cut -d: -f1)
    time=$(echo $item | cut -d: -f2)
    printf "${gb_file}: "
    [ -r "${testdir}/$gb_file" ] || echo "Warning: cannot find test file $gb_file in $testdir" 1>&2
    expected="$(basename "${testdir}/$gb_file" .gb)


Passed"
    status=
    "$exec" "${testdir}/$gb_file" ${time}000000 > $temp 2> $temp2
    if [ "x$(cat $temp)" = "x$expected" ]; then
        status=ok
    else
        status=ko
    fi
    [ "x$(cat $temp2)" = 'x' ] || status="${status}+stderr"
    [ "x$status" = 'xok' ] && echo "${green}PASSED${end}." || echo "${red}FAILED${end}."
    case $status in
        ko)        echo "  expected output is incorrect:"; display $temp ;;
        ko+stderr) echo "  expected output is incorrect:"; display $temp; echo "    and stderr is:"; cat $temp2 ;;
        ok+stderr) echo "  expected output is correct (thus seems test has passed)"; echo "    but got errors on stderr:"; cat $temp2 ;;
    esac
    echo "=========================================="
done
