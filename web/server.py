import subprocess
from flask import Flask, render_template, request


def save_query(query):
    with open('./src/queries/in', 'w', encoding='UTF-8') as file:
        file.write(query)


def run_search_engine():
    subprocess.call(['bash', './src/search.sh'])


def parse_results(results):
    with open('./src/results/out', 'r', encoding='UTF-8') as file:
        total = file.readline().rstrip()
        while (line := file.readline().rstrip()):
            title, url = line.split("\" \"")
            results[title[1:]] = url[:-1]


app = Flask(__name__)


@app.route('/')
def home():
    return render_template('index.html')


@app.route('/search', methods=['GET', 'POST'])
def search():
    query = request.form['query']
    save_query(query)

    run_search_engine()

    results = dict()
    parse_results(results)

    return render_template('search.html', query=query, results=results)


if __name__ == '__main__':
    app.run(debug=True)
