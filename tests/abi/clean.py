from pathlib import Path
R=Path(__file__).resolve().parent
for p in list(R.rglob("*.bin"))+list(R.rglob("*.log")): p.unlink()
print("ABI tests: cleaned")
