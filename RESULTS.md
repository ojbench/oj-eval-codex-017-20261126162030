# Evaluation log

- Submission 879422, commit 50a162b: score 67. All six basic groups and
  all six stress groups passed. Extended basic group timed out at test 40
  (15,982 ms). Peak memory was below 9 MiB.
- Follow-up: increase the train schedule cache from 256 to 8,192 entries
  (approximately 13 MiB) to avoid disk reads in repeated transfer searches.
- Submission 879426, commit 8193dd2: **Accepted**, reported score **75**.
  All 102 test points across all 13 groups passed. Peak memory: 21,516,288
  bytes; disk usage: 194,727,936 bytes; persistent file count: 5.
  Two of the six allowed submissions were used.

Local validation: persistence and overnight transfer integration tests, plus
1,500 randomized purchase/refund operations checked against an independent
seat model (`python3 tests/test_system.py`).
