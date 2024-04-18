pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                echo 'Building project'
                sh 'make clean && make'
            }
        }
        stage('Test') {
            steps {
                echo 'Running tests'
                //sh 'make tests'
            }
        }
        stage('Profiling'){
            steps {
                echo 'Profiling the program'
                sh 'make valgrind ARGS="-n 20 -alg GREEDY -v --to_file -seed 123"'

                // to add: performance profiling
            }
        }
        stage('Run') { 
            steps {
                echo 'Running on all files, check status on Telegram'
                sh 'pip3 install scripts/requirements.txt' 
                sh 'python3 scripts/compare_algs.py all'
                sh 'python3 scripts/perfprof.py results/all.csv results/all.pdf -D , -M 3' 
            }
        }
        stage('Webapp Build') {
            steps{
                echo 'Building web application'
                sh 'cd webapp'
                sh 'npm install --prefix client'
                sh 'npm install --prefix server'
            }
        }
        stage('Webapp Deploy') {
            steps{
                echo 'Deploying web application'
                sh 'npm run --prefix client/ dev'
                sh 'npm run --prefix server/ start'
            }
        }
    }
}