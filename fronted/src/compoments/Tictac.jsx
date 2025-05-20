import React, { useState, useEffect, useContext } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import { UserContext } from '../UserContext';

const Tictac = () => {
  const { state } = useLocation();
  const { sessionId } = state || {}; // Defensive: in case state is missing
  // const {currentPlayer} = state || {};
  const { username } = useContext(UserContext);
  const navigate = useNavigate();

  // Server‑driven state
  const [board, setBoard] = useState([
    ['', '', ''],
    ['', '', ''],
    ['', '', ''],
  ]);
  const [currentPlayer, setCurrentPlayer] = useState(null);
  const [winner, setWinner] = useState(null);
  const [ws, setWs] = useState(null);
  const [waiting, setWaiting] = useState(false);
  const [isFinished, setIsFinished] = useState(false);

  // ====== 1. Establish WebSocket and handlers ======
  useEffect(() => {
    if (!sessionId || !username) return;
    
    let backendHost = import.meta.env.VITE_API_URL || 'localhost:8080';
    backendHost = backendHost.replace(/^https?:\/\//, '');
    const protocol = window.location.protocol === 'https:' ? 'wss' : 'ws';
    const socket = new WebSocket(`${protocol}://${backendHost}/ws/game/${sessionId}`);
    console.log({socket})
    socket.onopen = () => {
      socket.send(JSON.stringify({ sessionId, username }));
      console.log("WebSocket connection established");
      // const state = JSON.parse(st.data);
      // setCurrentPlayer(state.currentPlayer);

    };

    socket.onmessage = (event) => {
      const state = JSON.parse(event.data);
      console.log("Received state:", state);

      if (state.waiting) {
        setWaiting(true);
        setCurrentPlayer(null);
        setWinner(null);
        setBoard([
          ['', '', ''],
          ['', '', ''],
          ['', '', ''],
        ]);
        return;
      }

      setWaiting(false);
      setBoard(state.board);
      setCurrentPlayer(state.currentPlayer);
      setWinner(state.winner || null);

      if (state.finished) {
        setIsFinished(true);
      }
    };
    console.log(currentPlayer)
    console.log({state})

    socket.onclose = () => {
      setWs(null);
    };

    setWs(socket);
    return () => {
      console.log("Tictac unmounted, closing WebSocket", socket.readyState);
      socket.close();

    }
  }, [sessionId, username]);

  // ====== 2. Handle user clicks & send moves ======
  const handleClick = (row, col) => {
    console.log('username:', username, 'currentPlayer:', currentPlayer);

    if (!ws || ws.readyState !== 1 || winner) {
      console.log('WebSocket is not open or game is over', {ws});
      return; // Only if open
    }
    if (board[row][col] !== '' || currentPlayer !== username) {
      console.log('Invalid move');
      return;
      
    }
    ws.send(JSON.stringify({ username, row, col }));
  };

  // ====== 3. If game over, optionally allow “Play Again” or redirect ======
  const handleRestart = () => {
    navigate('/battle_lobby', { state: { gameType: 'Tic Tac Toe', username } });
  };
  const handleMainLobby = () => {
    navigate('/lobby', { state: { username } });
  }

  // ====== 4. Render ======
  const renderCell = (r, c) => (
    <div
      key={`${r}-${c}`}
      style={{
        ...styles.cell,
        cursor: (board[r][c] === '' && currentPlayer === username && !winner)
          ? 'pointer' : 'not-allowed',
      }}
      onClick={() => handleClick(r, c)}
    >
      {board[r][c]}
    </div>
  );

  return (
    <div style={styles.container}>
      <h1 style={styles.heading}>🎮 Tic Tac Toe</h1>
      {waiting ? (
        <p style={styles.status}>Waiting for another player to join...</p>
      ) : (
        <p style={styles.status}>
          {winner
            ? `🏆 Winner: ${winner}`
            : board.flat().every(cell => cell !== '')
            ? "🤝 It's a Tie!"
            : currentPlayer === username
            ? 'Your move'
            : `${currentPlayer || '...'}'s move`}
        </p>
      )}

      <div style={styles.board}>
        {board.map((rowArr, r) =>
          rowArr.map((_, c) => renderCell(r, c))
        )}
      </div>

      {(winner || board.flat().every(cell => cell !== '')) && !waiting ? (
    <>
      <button onClick={handleRestart} style={styles.resetButton}>
        Play Again
      </button>
      <button onClick={handleMainLobby} style={{ ...styles.resetButton, backgroundColor: '#2ecc71', marginLeft: 10 }}>
        Main Lobby
      </button>
  </>
) : null}

    </div>
  );
};

const styles = {
  container: {
    textAlign: 'center',
    marginTop: '60px',
    fontFamily: 'Arial, sans-serif',
    color: '#2c3e50',
  },
  heading: {
    fontSize: '2.5rem',
    marginBottom: '10px',
  },
  status: {
    fontSize: '1.4rem',
    marginBottom: '20px',
  },
  board: {
    display: 'grid',
    gridTemplateColumns: 'repeat(3, 100px)',
    gridGap: '10px',
    justifyContent: 'center',
    marginBottom: '20px',
  },
  cell: {
    width: '100px',
    height: '100px',
    backgroundColor: '#ecf0f1',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '2rem',
    borderRadius: '10px',
    boxShadow: '0 2px 5px rgba(0,0,0,0.2)',
    transition: '0.2s',
  },
  resetButton: {
    padding: '10px 20px',
    fontSize: '1rem',
    borderRadius: '8px',
    backgroundColor: '#3498db',
    color: 'white',
    border: 'none',
    cursor: 'pointer',
  },
};

export default Tictac;
