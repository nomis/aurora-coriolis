.PHONY: clean

PIPENV_VENV_IN_PROJECT=1

.venv/bin/python: Pipfile
	pipenv install --skip-lock

clean:
	rm -rf .venv
