PYTHON		= python3
CONFIG		= config.txt
MAIN		= a_maze_ing.py
VENV		= .venv

install:
	$(PYTHON) -m pip install --upgrade pip
	$(PYTHON) -m pip install flake8 mypy build

run:
	$(PYTHON) $(MAIN) $(CONFIG)

debug:
	$(PYTHON) -m pdb $(MAIN) $(CONFIG)

build:
	$(PYTHON) -m build --wheel --outdir .
	rm -rf build *.egg-info mazegen.egg-info

clean:
	rm -rf __pycache__ */__pycache__ .mypy_cache .pytest_cache
	rm -rf build dist *.egg-info

lint:
	flake8 . --exclude $(VENV)
	mypy . --warn-return-any --warn-unused-ignores --ignore-missing-imports \
		--disallow-untyped-defs --check-untyped-defs --exclude $(VENV)

lint-strict:
	flake8 . --exclude $(VENV)
	mypy . --strict --exclude $(VENV)

.PHONY: install run debug build clean lint lint-strict
