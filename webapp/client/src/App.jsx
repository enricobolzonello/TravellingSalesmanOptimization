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

var nodes = [];
var links = [];

export default function App() {
  const [select_array, setSelectArray] = useState([]);

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

  const [processing, setProcessing] = useState(false);

  const onSubmit = (data) => {
    if (data.botField) {
      // It's likely a bot submission, handle accordingly
      console.log('Bot detected, submission ignored.');
      return;
    }

    setProcessing(true);
    axios.post('/run', data)
        .then(response => {
            nodes = response.data.points;
            links = response.data.links;
            // Handle success response if needed
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
          <Form select_array={select_array} onSubmit={onSubmit} processing={processing}/>
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
      
      <div className="bg-[#212121] rounded-3xl p-10 m-5 mb-0">
        <h3 className="font-black text-gray-100 uppercase text-l pb-5">Results</h3>
      </div>
    </div>
  );
}