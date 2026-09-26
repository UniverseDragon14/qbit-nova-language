#!/usr/bin/env bash
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
Q="$ROOT/build/qnova"
EX="$ROOT/examples"
OUT="$ROOT/tests/out"
pass=0
fail=0

ok() { printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad() { printf 'FAIL %s: %s\n' "$1" "$2"; fail=$((fail+1)); }
eq() {
  local id="$1" expected="$2" got="$3"
  if [ "$got" = "$expected" ]; then ok "$id"; else bad "$id" "expected=[$expected] got=[$got]"; fi
}
errcode() {
  local id="$1" code="$2" file="$3"
  local got
  got="$("$Q" check "$file" 2>&1 >/dev/null || true)"
  case "$got" in *"QNOVA-E$code "*) ok "$id";; *) bad "$id" "$got";; esac
}
runerr() {
  local id="$1" code="$2" file="$3"
  local got
  got="$("$Q" run "$file" --rng test:1 2>&1 >/dev/null || true)"
  case "$got" in *"QNOVA-E$code "*) ok "$id";; *) bad "$id" "$got";; esac
}
qbcerr() {
  local id="$1" code="$2" file="$3"
  local got
  got="$("$Q" run-qbc "$file" --rng test:1 2>&1 >/dev/null || true)"
  case "$got" in *"QNOVA-E$code "*) ok "$id";; *) bad "$id" "$got";; esac
}

eq EX001 "dragon" "$("$Q" run "$EX/ex001.qnova")"
eq EX002 "3" "$("$Q" run "$EX/ex002.qnova")"
eq EX003 "ok" "$("$Q" run "$EX/ex003.qnova")"
runerr EX004 7001 "$EX/ex004.qnova"
eq EX005 "2" "$("$Q" run "$EX/ex005.qnova")"
eq EX006 "1" "$("$Q" run "$EX/ex006.qnova")"
eq EX007 "p0=1.000,p1=0.000" "$("$Q" run "$EX/ex007.qnova")"
eq EX008 "p0=0.500,p1=0.500" "$("$Q" run "$EX/ex008.qnova")"
eq EX009 "1" "$("$Q" run "$EX/ex009.qnova" --rng test:1)"
eq EX010 $'p0=0.500,p1=0.500\np0=0.500,p1=0.500' "$("$Q" run "$EX/ex010.qnova" --rng test:1)"
errcode EX011 4003 "$EX/ex011.qnova"
errcode EX012 3001 "$EX/ex012.qnova"
errcode EX013 4001 "$EX/ex013.qnova"
errcode EX014 3003 "$EX/ex014.qnova"

"$Q" compile "$EX/ex015.qnova" -o "$OUT/ex015-a.qbc" || bad EX015_compile_a "compile failed"
"$Q" compile "$EX/ex015.qnova" -o "$OUT/ex015-b.qbc" || bad EX015_compile_b "compile failed"
ha="$(sha256sum "$OUT/ex015-a.qbc" | awk '{print $1}')"
hb="$(sha256sum "$OUT/ex015-b.qbc" | awk '{print $1}')"
if [ "$ha" = "$hb" ]; then ok EX015_deterministic_qbc; else bad EX015_deterministic_qbc "$ha != $hb"; fi
case "$("$Q" disasm "$OUT/ex015-a.qbc" | head -1)" in "QBC0 0.1"*) ok EX015_qbc_header;; *) bad EX015_qbc_header "bad header";; esac

eq FUNCTION "5" "$("$Q" run "$EX/function.qnova")"
eq NESTED "7" "$("$Q" run "$EX/nested.qnova")"
eq RECURSION "120" "$("$Q" run "$EX/recursion.qnova")"
eq REPEAT $'dragon\ndragon\ndragon' "$("$Q" run "$EX/repeat.qnova")"
eq WHEN "yes" "$("$Q" run "$EX/when.qnova")"
eq SIM_Q $'[sim] p0=0.000,p1=1.000\np0=1.000,p1=0.000' "$("$Q" run "$EX/sim_quantum.qnova" --rng test:1)"
eq GUARD_ELSE "denied" "$("$Q" run "$EX/guard_else.qnova")"
eq REAL "0.5" "$("$Q" run "$EX/real.qnova")"

"$Q" compile "$EX/function.qnova" -o "$OUT/function.qbc" || bad QBC_ROUNDTRIP_COMPILE "compile failed"
eq QBC_ROUNDTRIP "5" "$("$Q" run-qbc "$OUT/function.qbc" --rng test:1)"


# EXTENDED_ERROR_MODEL
errcode ERR1001 1001 "$EX/err1001.qnova"
errcode ERR1002 1002 "$EX/err1002.qnova"
errcode ERR1003 1003 "$EX/err1003.qnova"
errcode ERR1004 1004 "$EX/err1004.qnova"
errcode ERR2001 2001 "$EX/err2001.qnova"
errcode ERR2002 2002 "$EX/err2002.qnova"
errcode ERR2003 2003 "$EX/err2003.qnova"
errcode ERR2004 2004 "$EX/err2004.qnova"
errcode ERR2005 2005 "$EX/err2005.qnova"
errcode ERR3002 3002 "$EX/err3002.qnova"
errcode ERR3004 3004 "$EX/err3004.qnova"
errcode ERR3005 3005 "$EX/err3005.qnova"
errcode ERR3006 3006 "$EX/err3006.qnova"
errcode ERR3007 3007 "$EX/err3007.qnova"
errcode ERR3008 3008 "$EX/err3008.qnova"
errcode ERR4002 4002 "$EX/err4002.qnova"
errcode ERR4005 4005 "$EX/err4005.qnova"
runerr ERR4006 4006 "$EX/err4006.qnova"
runerr ERR6001 6001 "$EX/err6001.qnova"
runerr ERR6002 6002 "$EX/err6002.qnova"
runerr ERR6003 6003 "$EX/err6003.qnova"
runerr ERR6005 6005 "$EX/err6005.qnova"
errcode ERR7003 7003 "$EX/err7003.qnova"

"$Q" compile "$EX/ex001.qnova" -o "$OUT/const-base.qbc"
python3 "$ROOT/tests/make_bad_qbc.py"
qbcerr QBC5001 5001 "$OUT/bad-magic.qbc"
qbcerr QBC5002 5002 "$OUT/bad-version.qbc"
qbcerr QBC5003 5003 "$OUT/bad-const.qbc"
qbcerr QBC5004 5004 "$OUT/unknown-op.qbc"
qbcerr QBC5005 5005 "$OUT/truncated.qbc"
qbcerr QBC5006 5006 "$OUT/bad-crc.qbc"
printf '\nRESULT pass=%d fail=%d\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
