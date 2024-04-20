import axios from "axios";
import { useState, useEffect } from 'react';

import NetworkGraph from "./components/Plot";
import Form from "./components/Form"
import Description from "./components/Description";
import { useParentSize } from '@visx/responsive';

import {
  PanelGroup,
  Panel,
  PanelResizeHandle
} from "react-resizable-panels";

import tsp_solutions from "./assets/solutions";

var nodes = [];
var links = [];

export default function App() {

  const [select_array, setSelectArray] = useState([]);

  // fetch all datasets
  useEffect(() => {
    fetchData();
  }, []);

  const fetchData = async () => {
    try {
      const response = await axios.get('/palle');

      setSelectArray(response.data);
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  };

  const [ cost, setCost ] = useState('0');
  const [ executionTime, setExecutionTime ] = useState('0');
  const [ fromOptimal, setFromOptimal ] = useState('0');
  const [ processing, setProcessing ] = useState(false);

  function calculateFromOptimal(temp, dataset){
      const cost_int = Number(temp);
      const optimal_int = Number(tsp_solutions[dataset]);

      const percentage = ( Math.abs(cost_int - optimal_int) / ((cost_int+optimal_int) / 2) ) * 100;

      setFromOptimal(percentage.toFixed(2));
  }

  const onSubmit = (data) => {
    if (data.botField) {
      // It's likely a bot submission, handle accordingly
      console.log('Bot detected, submission ignored.');
      return;
    }

    setProcessing(true);
    axios.post('/run', data)
        .then(response => {
            if(response.data.length != 0){
              nodes = response.data.points;
              links = response.data.links;

              const temp = response.data.cost.toFixed(0);
              setCost(temp);
              setExecutionTime(response.data.execution_time.toFixed(4));

              calculateFromOptimal(temp, response.data.filename);
            }
            
            setProcessing(false);
        })
        .catch(error => {
            console.error('Error:', error);
            setProcessing(false);
        });
  };

  const { parentRef, width, height } = useParentSize({ debounceTime: 0 });

  return (
    <div className="bg-[#121212]">
      <div className="h-screen">
      <PanelGroup direction="horizontal">
        <Panel defaultSize={30} minSize={20} style={{overflow: 'scroll'}}>
        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-2">
          <Description />
        </div>

        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-2">
        {select_array.length > 0 && <Form select_array={select_array} onSubmit={onSubmit} processing={processing}/>}
        </div>
        </Panel>
        <PanelResizeHandle className="w-1	hover:border-2 hover:border-solid hover:border-[#21d4fc] active:border-2 active:border-solid active:border-[#21d4fc]" />
        <Panel defaultSize={70} minSize={50}>
        <div ref={parentRef} className="h-full m-5 ml-2">
          <NetworkGraph width={width} height={height} nodes_s={nodes} links_s={links}/>
        </div>
        </Panel>
      </PanelGroup>
      </div>
      
      <div className="grid grid-cols-4 grid-rows-2">
        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-0 mb-0 col-span-1">
          <div className="flex">
          <h3 className="font-medium text-gray-400 uppercase text-m pb-5 flex-1">Calculated Cost</h3>
          <svg className="w-6 h-6 text-gray-400 dark:text-white" aria-hidden="true" xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="none" viewBox="0 0 24 24">
            <path stroke="currentColor" strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="m8 8-4 4 4 4m8 0 4-4-4-4m-2-3-4 14"/>
          </svg>
          </div>
          <p className="mb-2 text-3xl font-extrabold text-gray-100">{cost}</p>
          <p className="text-gray-400"><span className="text-[#21d4fc] font-bold">{fromOptimal}%</span> from the optimal</p>
        </div>

        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-0 mb-0 col-span-1">
        <div className="flex">
          <h3 className="font-medium text-gray-400 uppercase text-m pb-5 flex-1">Execution Time</h3>
          <svg className="w-6 h-6 text-gray-400 dark:text-white" aria-hidden="true" xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="none" viewBox="0 0 24 24">
            <path stroke="currentColor" strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 8v4l3 3m6-3a9 9 0 1 1-18 0 9 9 0 0 1 18 0Z"/>
          </svg>
        </div>
        <p className="mb-2 text-3xl font-extrabold text-gray-100">{executionTime}</p>
        <p className="text-gray-400">seconds</p>
        </div>

        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-0 mb-0 col-span-1 row-start-2">
          <h3 className="font-medium text-gray-400 uppercase text-m pb-5">Results</h3>
        </div>
        <div className="bg-[#212121] rounded-3xl p-10 m-5 mr-0 mb-0 col-span-1 row-start-2">
          <h3 className="font-medium text-gray-400 uppercase text-m pb-5">Results</h3>
        </div>
      <div className="bg-[#212121] rounded-3xl p-10 m-5 mb-0 col-span-2 row-span-2">
        <h3 className="font-medium text-gray-400 uppercase text-m pb-5">Cost during Iterations</h3>
      </div>
      </div>
    </div>
  );
}