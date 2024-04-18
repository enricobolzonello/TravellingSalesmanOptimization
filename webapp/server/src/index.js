const TSPModule = require('../build/Release/tspsolver.node');
const express = require("express");
const cors = require("cors");
const fs = require('fs');
const path = require('node:path'); 

const PORT = process.env.PORT || 8080;
const app = express();

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const dataArray = [1, 2, 3, 4, 5];

app.get("/palle", (req, res) => {
    // Find all TSPLIB dataset in the appropriate directory
    const directory = "../../data/test/";
    const directory_content = fs.readdirSync(directory);
    let tsp_files = directory_content.filter( ( elm ) => elm.match(/.*\.(tsp?)/ig));

    let select_array = []
    for(let i=0; i < tsp_files.length; i++){
        let temp_object = {
            "value" : directory + tsp_files[i],
            "label" : path.basename(tsp_files[i], '.tsp'),
        }

        select_array.push(temp_object)
    }

    res.status(200).send(select_array);
})

app.post("/run", (req, res) => {
    const algorithm = Number(req.body.algorithm);
    const seed = Number(req.body.seed);
    const timelimit = Number(req.body.timelimit);
    const dataset = req.body.dataset;

    console.log("running TSP with alg " + algorithm + ", seed: " + seed + ", timelimit: " + timelimit + ", dataset: " + dataset);

    let obj = TSPModule.TSP_runner(dataset, seed, timelimit, algorithm)

    console.log('cost : ', obj['cost']);
    console.log();

    console.log("nnodes: ", obj['nnodes']);
    console.log();

    const linksArray = [];
    for(let i=0; i<obj['nnodes']; i++){
        let temp = {
            "source" : obj['points'][i],
            "target" : obj['points'][obj['path'][i]]
        }
        linksArray.push(temp);
    }

    res.json({
        points : obj['points'],
        links : linksArray
    });
});

// start the Express server
app.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}`);
});