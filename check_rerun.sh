export LC_ALL=C
export LANG=C
cd /root/judge
kill `cat jr.pid`
./judgerunner.py
