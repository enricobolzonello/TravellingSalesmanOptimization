import { useForm } from "react-hook-form";
import PropTypes from 'prop-types';
import { useState } from 'react';

import '../output.css'

Form.propTypes = {
    select_array: PropTypes.array,
    onSubmit: PropTypes.func,
    processing: PropTypes.bool
}


function Form({select_array ,onSubmit, processing }) {

    const {
        register,
        handleSubmit,
        setValue
    } = useForm()

    const [selectedValue, setSelectedValue] = useState(""); // Initialize with empty string or any default value

    return (
        <form
            className="w-full max-w-lg"
            onSubmit={handleSubmit(onSubmit)}
        >
            <div className="space-y-5">
                {/* algorithm selector*/}
                <div className="flex flex-wrap">
                    <div className="w-full px-3">
                        <label className="block uppercase tracking-wide text-gray-100 text-xs font-bold mb-2">ALGORITHM</label>
                        <select
                            className="block appearance-none w-full bg-gray-200 border border-gray-200 text-gray-700 py-3 px-4 pr-8 rounded leading-tight focus:outline-none focus:bg-white focus:border-gray-500"
                            name="algorithm"
                            {...register("algorithm", { required: true })}
                        >
                            <option value="0">Greedy</option>
                            <option value="1">Greedy Iterative</option>
                            <option value="2">Greedy 2-opt</option>
                            <option value="3">Tabu Search</option>
                            <option value="4">VNS</option>
                            <option value="5">Cplex No SEC</option>
                            <option value="6">Benders Loop</option>
                            <option value="7">Extra Mileage</option>
                            <option value="8">Benders Loop with Patching</option>
                        </select>
                        <p id="helper-text-explanation" className="mt-2 text-xs text-gray-500 dark:text-gray-400">Which algorithm will be used to solve the instance </p>
                    </div>
                </div>

                {/* seed and time limit*/}
                <div className="flex flex-wrap">
                    <div className="w-full md:w-1/2 px-3 mb-6 md:mb-0">
                        <label className="block uppercase tracking-wide text-gray-100 text-xs font-bold mb-2">Seed</label>
                        <input
                            className="appearance-none block w-full bg-gray-200 text-gray-700 border border-gray-200 rounded py-3 px-4 leading-tight focus:outline-none focus:bg-white focus:border-gray-500"
                            type="number"
                            name="seed"
                            defaultValue="-1"
                            min="-1"
                            {...register("seed", { required: true })}
                        />
                        <p id="helper-text-explanation" className="mt-2 text-xs text-gray-500 dark:text-gray-400"> Seed for random operations </p>
                    </div>
                    <div className="w-full md:w-1/2 px-3">
                        <label className="block uppercase tracking-wide text-gray-100 text-xs font-bold mb-2">Time Limit</label>
                        <input
                            className="appearance-none block w-full bg-gray-200 text-gray-700 border border-gray-200 rounded py-3 px-4 leading-tight focus:outline-none focus:bg-white focus:border-gray-500"
                            type="number"
                            name="timelimit"
                            defaultValue="-1"
                            min="-1"
                            {...register("timelimit", { required: true })}
                        />
                        <p id="helper-text-explanation" className="mt-2 text-xs text-gray-500 dark:text-gray-400">Time limit in seconds for the total computation time </p>
                    </div>
                </div>

                {/* Dataset selector */}
                <div className="flex flex-wrap">
                    <div className="w-full px-3">
                        <label className="block uppercase tracking-wide text-gray-100 text-xs font-bold mb-2">Dataset</label>
                        {/*<select
                            className="block appearance-none w-full bg-gray-200 border border-gray-200 text-gray-700 py-3 px-4 pr-8 rounded leading-tight focus:outline-none focus:bg-white focus:border-gray-500"
                            name="dataset"
                            {...register("dataset")}
                        >
                            <option
                                value="../../data/berlin52.tsp"
                            
                            >
                                berlin52
                            </option>
                            <option
                                value="../../data/a280.tsp"
                                
                            >
                                a280
                            </option>
                        </select>*/}

                        <select
                            className="block appearance-none w-full bg-gray-200 border border-gray-200 text-gray-700 py-3 px-4 pr-8 rounded leading-tight focus:outline-none focus:bg-white focus:border-gray-500"
                            name="dataset"
                            value={selectedValue} // Set the value of the select to the selectedValue state
                            onChange={(e) => {
                                setSelectedValue(e.target.value); // Update selectedValue state when an option is selected
                                setValue("dataset", e.target.value); // Update React Hook Form value
                            }}
                        >
                            {select_array.map(option => (
                                <option key={option.value} value={option.value}>
                                    {option.label}
                                </option>
                            ))}
                        </select>
                        <p id="helper-text-explanation" className="mt-2 text-xs text-gray-500 dark:text-gray-400"> TSPLIB instance (limited to Symmetric TSP and Euclidean distance) </p>
                    </div>
                </div>

                {/* Honey Pot */}
                <input type="text" style={{ display: 'none' }} {...register('botField')} />

                </div>

                <button
                    type="submit"
                    className={`inline-block mt-10 py-2 px-4 mx-3 rounded-lg bg-[#21d4fc] text-white font-semibold relative ${processing ? 'opacity-50 cursor-not-allowed' : 'hover:bg-sky-600'
                        }`}
                    disabled={processing}
                >
                    {processing ? (
                        <div className="flex items-center">
                            <svg className="animate-spin -ml-1 mr-3 h-5 w-5 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                                <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                                <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                            </svg>Processing...
                        </div>
                    ) : (
                        'Run'
                    )
                    }
                </button>
        </form>
    );
}

export default Form;