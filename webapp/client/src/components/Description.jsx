import "../output.css"


function Description(){
    return (
        <>
            <h1 className="font-black text-gray-100 uppercase text-xl pb-5">TSP Solver</h1>
            <p className="text-gray-400">
                Solver for the Travelling Salesman Problem with the algorithms saw in the Operation Research 2 course at the University of Padua.  
            </p>
            <p className="text-gray-400">
            All the algorithms are implemented in C while the webapp is done in Node.JS as backend and React as frontend.
            </p>
        </>
    );
}

export default Description;