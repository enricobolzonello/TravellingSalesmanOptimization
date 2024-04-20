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
            <br />
            <p className="text-gray-400">
                All the code can be found on Github. 
            </p>
            <a href="https://github.com/enricobolzonello/TravellingSalesmanOptimization" className="inline-flex items-center font-medium text-[#21d4fc] hover:underline">
                GitHub Repository<svg className="w-4 h-4 ms-2 rtl:rotate-180" aria-hidden="true" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 14 10">
                    <path stroke="currentColor" strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M1 5h12m0 0L9 1m4 4L9 9"/>
                </svg>
                </a>
        </>
    );
}

export default Description;